#C:\cmd\mem\mem.py

import os
import optparse
import sys
import re
import random
import pprint
from progress_bar import ProgressBar
import shutil
import operator
import md5
import hashlib
from sets import Set    # built in for 2.6 when upgrade
import locale
import struct
import sqlite3
import gc

__version__ = '0.1'

# @todo might want to try Cython for faster accelleration
try:
  import psyco
  psyco.full()
  # print>>sys.stderr, "psyco acceleration"
except ImportError:
  print>>sys.stderr, "no psyco acceleration"
  pass

locale.setlocale(locale.LC_ALL, "")


class MemTrack:
  # some values and options for controlling analysis and reporting
  event_log_read_limit = 0x7FFFFFF
  reporting_size_limit = 0
  prune_callstacks = 1
  call_stack_write_limit = -1
  pass

class Address:
  def __init__(self):
      self.address = 0
      self.name = "<NULL>"
      self.displacement = 0
      self.filename = ""
      self.linenumber = 0
      self.name_loc = "<NULL>"
      self.file_loc = ""
      
  def __str__(self):
    s = "%s (%s+%s) %s(%d)"%(self.name, hex(self.address), hex(self.displacement), self.filename, self.linenumber)
    return s
  
  def prepare(self):
    self.name_loc = "%s+%s"%(self.name,hex(self.displacement))
    self.file_loc = "%s(%s)"%(self.filename,str(self.linenumber))

# wrapper around address symbol info sqlite database
class SymbolDB:
  symbol_cache = {}     # symbol cache shared by all instances

  def __init__(self, dbpath, verbose=False):
      self.dbpath = dbpath
      self.connection = None
      self.verbose = verbose
      
  def connect(self):
    if not self.connection:
      self.connection = sqlite3.connect( self.dbpath )

  def lookup(self, key):
      result = None
      cu = self.connection.cursor()
      sql_query = "select * from symbols where address= '%s'"
      sql = sql_query%key
      try:
          cu.execute( sql )
          result = cu.fetchone()
      except sqlite3.Error, errmsg:
          print "Could not execute the query: " + str(errmsg)
      return result
      
   # given an address object lookup the address and populate the symbol information
  def resolve( addr_obj ):
    result = lookup( addr_obj.address )
    if result:
      addr_obj.name = result[1]
      addr_obj.displacement = result[2]
      addr_obj.filename = result[3]
      addr_obj.linenumber = result[4]
    return

# we don't have that many unique addresses in all the event code points
# so lets resolve them all from the symbol db at once an cache them in
# a dictionary for quicker access
def build_address_symbol_cache( fname_address_db ):
  symbol_cache = {}
  
  # seed with the "NULL" address
  addr_obj = Address()
  symbol_cache[ addr_obj.address ] = addr_obj
  
  connection = sqlite3.connect( fname_address_db )
  cu = connection.cursor()
  sql_query = "SELECT * FROM symbols"
  try:
    cu.execute( sql_query )
    #iterate over each row and add to our cache
    for row in cu:
      addr_obj = Address()
      addr_obj.address      = row[0]
      addr_obj.name         = row[1]
      addr_obj.displacement = row[2]
      addr_obj.filename     = row[3]
      addr_obj.linenumber   = row[4]
      addr_obj.prepare()  # generate name variant fields
      symbol_cache[ addr_obj.address ] = addr_obj
  except sqlite3.Error, errmsg:
    print "Could not execute the query: " + str(errmsg)
  connection.close()        
  print "address symbols cached: %d"%len(symbol_cache)
  return symbol_cache
  
# callstack object is a hash and a list of addresses
class Callstack:
  instrument_names = [
    "new", "new[]", "delete", "delete[]", "malloc", "calloc", "free", "realloc", "_calloc_dbg", "_malloc_dbg", "_nh_malloc_dbg",
    "memtrack_log_callstack", "memtrack_hook_func", "memtrack_log_memory_event", "memtrack_alloc", "memtrack_free", "memtrack_realloc", "MemMonitorHook",
    "_heap_alloc_dbg", "free_timed", "free", "malloc_timed", "calloc_timed", "strdup_timed", "_strdup_dbg",
    "_free_dbg", "_free_dbg_nolock", "realloc_timed", "_realloc_dbg", "realloc_help",
    "_wchartodigit",
    "memtrack_do_malloc", "memtrack_do_realloc", "memtrack_do_calloc", "memtrack_do_free",
    #todo We don't currently track the small block heap allocators separately (as children of the global heap) so prune the instrumentation up past the memory pool functions to get more meaningful reports for the time being
    "initArray_dbg", "arrayPushBack_dbg", "mpAddMemoryChunk_dbg", "mpAlloc_nolock", "mpAlloc_dbg", "mallocPoolMemory", "eaCreateWithCapacityDbg"
    ]

  def __init__(self, hash, display_id):
    self.calls = []
    self.hash = hash                # the hash value from the dictionary
    self.display_id = display_id    # a more human readable id/name for this callstack
    self.leaf_index = 0             # the call index we want to be the leaf/end of callstack,  defaults to first call index

  # return the address that we consider to be the end/leaf of the callstack
  # generally we try to bump this up past the instrumentation and allocator functions to make 
  # reporting a little easier/nicer
  def leaf(self):
    leaf = None
    if len(self.calls) and self.leaf_index >= 0:
      leaf = self.calls[self.leaf_index]
    return leaf
  
  def prune(self):
    # prune the stack to remove the leaf calls that are part of the instrumentation and recursion
    # makes for more informative report generation
    self.leaf_index = -1
    for index, call in enumerate(self.calls):
      try:
        call_info = SymbolDB.symbol_cache[ call ]
        instrument_index = self.instrument_names.index( call_info.name )
      except ValueError:
        self.leaf_index = index
        pass
      if self.leaf_index >= 0:
        break
    return
    
  def prepare(self):
    # prepare callstack by pruning instrumentation calls
    if MemTrack.prune_callstacks:
      self.prune()
    
  def html_table(self):
    #generate the html callstack table to embed in summary table
    stack_html = '<div><span class="cl" onClick="utog(this)" title="expand for full callstack"> <a name="%s">Callstack %s &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; hash[%s]</a>'%(self.hash, self.display_id, self.hash)
    stack_html += """<table class="cl" title="call stack">
                  %s
                  </table>
                  </span></div>
                  """     
    stack_rows = ""
    # if we can back up one call off of the 'non instrumented' leaf to show what the allocator function was
    start_index = self.leaf_index
    if self.leaf_index > 0:
      start_index -= 1
    for index in range(start_index,len(self.calls)):
      call = self.calls[index]
      call_info = SymbolDB.symbol_cache[ call ]
      stack_rows += "<tr><td>%s</td><td>%s</td></tr>" % (call_info.name_loc, call_info.file_loc)
    callstack_html = stack_html % stack_rows
    return callstack_html

class Event:
  (eHook, eAlloc, eRealloc, eFree, eMarker) = range(0,5)
  eAny = 0xFF
  
  def text(self):
    payload_text = ""
    if self.event_type == Event.eAlloc:
      tName = "alloc"
      payload_text = "pData: %s size: %d"%(hex(self.pData), self.allocSize)
    elif self.event_type == Event.eRealloc:
      tName = "realloc"
      payload_text = "pData: %s size: %d pDataPrev: %s"%(hex(self.pData), self.allocSize, hex(self.pDataPrev))
    elif self.event_type == Event.eFree:
      tName = "free"
      payload_text = "pData: %s"%(hex(self.pData))
    elif self.event_type == Event.eMarker:
      tName = "marker"
      payload_text = "pData: %d %s"%self.marker_type, self.marker_string
  
    fmt = "[%-8s, %8d, %s, %s]: %s" 
    s = fmt%(tName, self.request_id, hex(self.stack_id), hex(self.thread_id), payload_text )
    return s

class Accumulator:
  # tracks a varying value along with stats the events which  modify that value
  
  def __init__(self):
    self.histogram = {}
    self.value = 0
    self.value_max = -0x7FFFFFF
    self.value_min = 0x7FFFFFF
    self.added = 0
    self.removed = 0
    self.count_adds = 0
    self.count_removes = 0
    self.delta_max = -0x7FFFFFF
    self.delta_min = 0x7FFFFFF
    
  def add( self, size ):
    self.count_adds += 1
    self.added += size
    self.value += size
    if self.value > self.value_max : self.value_max = self.value
    if self.value < self.value_min : self.value_min = self.value
    self.histogram[ size ] = self.histogram.get( size, 0 ) + 1
    if size > self.delta_max : self.delta_max = size
    if size < self.delta_min : self.delta_min = size

  def remove( self, size ):
    self.removed += size
    self.value -= size
    self.count_removes += 1
    
  def replace( self, size_old, size_new ):
    if size_old:
      self.remove(size_old)
    if size_new:
      self.add(size_new)

# play events through a simulated heap
class ActiveBlock:
  pass
  
class Heap:
  (eError_NotInHeap, eError_Null) = range(0,2)
  
  def __init__(self):
    self.accumulator = Accumulator()
    self.blocks = {}
    self.current_event = None
    self.event_errors = []
    
  def block_size( self, pData ):
    size = 0
    if pData != 0:
      if self.blocks.has_key( pData ):
        block = self.blocks[ pData ]
        size = block.size
      #~ else:
        #~ print "ERROR: memory block not found in heap: %s"%hex(pData)
    return size
    
  def alloc( self, pData, size ):
    if pData == 0:
        self.event_errors.append( ( self.current_event, Heap.eError_Null ) )
    block = ActiveBlock()
    block.size = size
    block.originating_event =  self.current_event
    self.blocks[ pData ] = block # track the memory block
    self.accumulator.add( size )

  def free( self, pData ):
    if pData != 0:
      # look up the block
      if self.blocks.has_key( pData ):
        size = self.block_size( pData )
        self.accumulator.remove( size )
        del self.blocks[pData] #remove from blocks
        
        #annotate this event with the amount free'd
        if self.current_event.event_type == Event.eFree:
          self.current_event.allocSize = size
      else:
        self.event_errors.append( ( self.current_event, Heap.eError_NotInHeap ) )

  def realloc( self, pData, size, pDataPrev ):
    # first free the old block
    self.free( pDataPrev )
    # now track the new block
    self.alloc( pData, size )

  def track( self, event ):
    # sample the information in the event and add it to the summary
    self.current_event = event # store for error tracking/loggng
    if event.event_type == Event.eAlloc:
      self.alloc( event.pData, event.allocSize )
    elif event.event_type == Event.eRealloc:
      self.realloc( event.pData, event.allocSize, event.pDataPrev )
    elif event.event_type == Event.eFree:
      self.free( event.pData )
  
  def errors(self):
    return self.event_errors
    
class CallStatistics:
  report_abs_limit = 0  # absolute size limit that we bother with in traversing the hierarchy depth when reporting
  
  # tracks the stats for a call in a callstack
  # is a tree for hierarchical state
  def __init__(self, callInfo ):
    self.address = callInfo
    self.track_alloc = Accumulator()
    self.track_free = Accumulator()
    self.track_alloc_children = Accumulator()
    self.track_free_children = Accumulator()
    self.key = ""
    self.children = {}
    
  def sample_self_event( self, size_alloc, size_free ):
    # sample the information in the event and add it to the summary
    self.track_alloc.replace( size_free, size_alloc )

  def sample_child_event( self, size_alloc, size_free ):
    # sample the information in the event and add it to the summary
    self.track_alloc_children.replace( size_free, size_alloc )
  
  def add_child_call( self, call ):
    # add a new stats object for this call to our children if we don't have one already
    call_stats = self.children.setdefault( call, CallStatistics( call ) )    # lookup the stats for this call, or add new
    # we don't update the child's stats yet because that will get taken on the next call in the stack or explicitly for a leaf call
    return call_stats

  def dump_tree( self, depth=0 ):
    # recursively dump this tree to the output
    print '  '*depth,
    print self.func + ' ',
    print str(self.track_alloc.value) + ' ',
    print str(self.track_alloc.count_adds) + ' ',
    print str(self.track_alloc_children.value) + ' ',
    print str(self.track_alloc_children.count_adds) + ' ',
    print
    for child in self.children.values():
      child.dump_tree( depth + 1 )
    
def generate_hierarchy( event_list, stack_dict ):
  prog = ProgressBar(0, len(event_list), 77, mode='fixed', char='#', autoprint=True)
  #create a hierarchal track of the allocation event
  root = CallStatistics( 0 )
    
  #create a heap to track live allocated blocks and current memory status overall
  heap = Heap()
  for event in event_list:
#    print event.text()
    
    # if the event is a free then we need to find out the size of the block we are freeing
    # @todo this could be part of the event stream when using debug CRT but probably not for release builds?
    if event.event_type == Event.eFree:
      size_free = heap.block_size( event.pData )
    elif event.event_type == Event.eRealloc:
      size_free = heap.block_size( event.pDataPrev )
    else:
      size_free = 0
      
    if event.event_type == Event.eAlloc or event.event_type == Event.eRealloc:
      size_alloc = event.allocSize
    else:
      size_alloc = 0
      
    # only now that we have our size information on potential free blocks do we
    # let heap track event (since the block in question might get freed)
    heap.track( event )

    # start at the tree root and walk each stack call in this event and tally the event with this call and it's callee's
    stat_obj = root
    # start at the first call and walk down until we hit the desired leaf
    stack = stack_dict[event.stack_id]
    for index in range(len(stack.calls)-1, stack.leaf_index-2, -1):
      call = stack.calls[index]
      child_obj = stat_obj.add_child_call( call )
      # add stats for the child to the parent      
      stat_obj.sample_child_event( size_alloc, size_free )
      if index == stack.leaf_index-1:
        # add leaf stats on the child leaf node
        child_obj.sample_self_event( size_alloc, size_free )
      stat_obj = child_obj  # step down the tree for next call
    prog.increment_amount()
  print "\n\n"
  return root

class Summary:
  def __init__(self):
    self.accumulator = Accumulator()
    
  def __str__(self):
      s = ""
      for key, value in self.__dict__.iteritems():
            s += "%s: %s, "%(key, value)
      return s
  
  def sample( self, cmd, stack ):
    # sample the information in the log action and add it to the summary
 #   if cmd.__dict__.has_key("allocSize"):
    size = cmd.allocSize
    self.accumulator.add(size)
    self.stack_hash = stack.hash
    self.stack_display_id = stack.display_id
    #resolve stack leaf function symbols
    if SymbolDB.symbol_cache.has_key(stack.leaf()):
      addr_info = SymbolDB.symbol_cache[ stack.leaf() ]
      self.name = addr_info.name
      self.file_loc = addr_info.file_loc
      self.func_name = addr_info.name
      self.filename = addr_info.filename
      self.line = addr_info.linenumber
      self.name_loc = addr_info.name_loc
    else:
      self.name = "Unknown"
      self.file_loc = "Unknown"
      self.func_name = "Unknown"
      self.filename = "Unknown"
      self.line = -1
      self.name_loc = "Unknown"
      
  def prepare(self, all_total):
    # copy accumulator fields to new fields on us to make easier for table lookup/insertion
 #   for key, value in self.accumulator.__dict__:
 #     self.__dict__[key] = value
    self.value = self.accumulator.value
    self.value_max = self.accumulator.value_max
    self.value_min = self.accumulator.value_min
    self.added = self.accumulator.added
    self.removed = self.accumulator.removed
    self.count_adds = self.accumulator.count_adds
    self.count_removes = self.accumulator.count_removes
    self.delta_max = self.accumulator.delta_max
    self.delta_min = self.accumulator.delta_min

    self.stack_anchor = '<a href="#%s">%5d</a>'%(self.stack_hash, self.stack_display_id)
    self.delta_avg = self.accumulator.value/self.accumulator.count_adds
    self.overhead = self.accumulator.count_adds*36 # win32 CRT debug library block overhead
    self.hist_count = len(self.accumulator.histogram)
    if all_total:
      self.percentage = self.accumulator.value*100.0/all_total
    else:
      self.percentage = 0
    
      
def summarize( cmds, key, type, stack_dict ):
  #create a summary list based off of accumulating stats based on the given key
  prog = ProgressBar(0, len(cmds), 77, mode='fixed', char='#', autoprint=True)
  all_total = 0
  summary_dict = {}
  null_summary_key = CallStatistics(0)
  for cmd in cmds:
    if type == Event.eAny or cmd.event_type == type:
      size = cmd.allocSize
      all_total += size

      stack = stack_dict.get( cmd.stack_id )    # lookup the stack for this event
      # check stack for key first
      if stack.__dict__.has_key( key ):
        summary_key = stack.__dict__[ key ]
      elif SymbolDB.symbol_cache.has_key(stack.leaf()):
        addr_info = SymbolDB.symbol_cache[ stack.leaf() ]
        summary_key = addr_info.__dict__[ key ]
      else:
        summary_key = null_summary_key
      summary = summary_dict.setdefault( summary_key, Summary() )
      summary.sample( cmd, stack )
      summary.key = summary_key
    prog.increment_amount()
  print "\n\n"
  
  summaries_sorted = sorted(summary_dict.items(), key=lambda (k,v): v.accumulator.value, reverse=True )
  
  # after all the summaries are collected go ahead and generate some of the
  # final stats in each summary
  summaries = [ summary[1] for summary in summaries_sorted ]

  for summary in summaries:
    summary.prepare(all_total)
  return summaries

def build_callstack_detail_html( stack_dict ):
  html = ""
  prog = ProgressBar(0, len(stack_dict), 77, mode='fixed', char='#', autoprint=True)
  stacks_sorted = sorted( stack_dict.values(), key=operator.attrgetter('display_id') )
  for i, stack in enumerate(stacks_sorted):
    if MemTrack.call_stack_write_limit > 0 and i > MemTrack.call_stack_write_limit:
      break
    html += stack.html_table()
    html +="\n\n"
    prog.increment_amount()
  return html
  

def read_callstack_dictionary( fname ):
  stack_dict = {}
  f_dict = open(fname, "rb" )
  
  s_int = struct.Struct('I');
    
  version, = s_int.unpack(f_dict.read(4))
  print "version: %d"%version
  
  s = f_dict.read(4)
  prog = ProgressBar(0, os.path.getsize(fname), 77, mode='fixed', char='#', autoprint=True)
  while len(s) == 4:
    hash, = s_int.unpack(s)
    num_frames, = s_int.unpack(f_dict.read(4))
    stack_id = len(stack_dict)+1 # use to make more human readable callstack names in the report
    stack = Callstack(hash, stack_id)
    for i in range(num_frames):
      address, = s_int.unpack(f_dict.read(4))
      stack.calls.append( address );
    stack.prepare()
    stack_dict[ hash ] = stack
    s = f_dict.read(4)

    prog.update_amount(f_dict.tell())
    
  print "\n"  
  print "callstacks: %d"%len(stack_dict)
  f_dict.close();
  return stack_dict
  
def string_to_int( somestring ):
  somestring_text = "%s"%somestring
  return int(somestring_text)

def read_event_log( fname ):
  event_list = []
  fin = open(fname, "rb" )
  
  s_int = struct.Struct('I');
  version, = struct.unpack('I', fin.read(4))
  print "version: %d"%version
  
  #~ typedef struct
  #~ {
    #~ unsigned int event_size;		// size of complete chunk (header + payload)
    #~ unsigned int event_type;		// type of payload
    #~ unsigned int request_id;	// sequence number of the event or id
    #~ unsigned int stack_id;		// event callstack hash
    #~ unsigned int thread_id;		// id of thread causing the event
  #~ } MemTrackEvent_Header;
  s_header = struct.Struct('IIIII');
  
  prog = ProgressBar(0, os.path.getsize(fname), 77, mode='fixed', char='#', autoprint=True)
  
  hook_event_count = 0
  unknown_event_count = 0
  count = 0;
  s = fin.read( s_header.size )
  while len(s) == s_header.size and len(event_list) < MemTrack.event_log_read_limit:
    count += 1
    header = s_header.unpack( s )

    event = Event()
    event.event_size   = header[0]
    event.event_type   = header[1]
    event.request_id  = header[2]
    event.stack_id    = header[3]
    event.thread_id   = header[4]
   
    if event.event_type == Event.eAlloc:
      #~ MemTrackEvent_Alloc
      #~ void*			pData;
      #~ size_t			allocSize;
      event.pData,      = s_int.unpack(fin.read(4))
      event.allocSize,  = s_int.unpack(fin.read(4))
      event_list.append( event )
    elif event.event_type == Event.eRealloc:
      #~ MemTrackEvent_Realloc
      #~ void*			pData;
      #~ size_t			allocSize;
      #~ void*			pDataPrev;		// used by realloc to link to original allocation
      event.pData,      = s_int.unpack(fin.read(4))
      event.allocSize,  = s_int.unpack(fin.read(4))
      event.pDataPrev,  = s_int.unpack(fin.read(4))
      event_list.append( event )
    elif event.event_type == Event.eFree: # free
      #~ MemTrackEvent_Free
      #~ void*			pData;
      event.pData,      = s_int.unpack(fin.read(4))
      
      #todo
      # it  is useful to for 'free' blocks to also have an 'allocSize" field that represents how much
      #memory it was freeing (this could be part of the capture stream for CRT debug captures)
      # in place of that we annotate this event with the amount free'd during the simulation phase
      event.allocSize = 0
      
      event_list.append( event )
    elif event.event_type == Event.eMarker:
      #~ MemTrackEvent_Marker
      #~ int		marker_type
      #~ char *		marker_string
      event.marker_type,   = s_int.unpack(fin.read(4))
      marker_string = ""
      event.marker_string = ""
      while True:
        char = fin.read(1)
        if char == "\0":
          break
        marker_string += char
      event.marker_string = marker_string
      event_list.append( event )
    elif event.event_type == Event.eHook:
      #@todo hook events are used for debugging and validation
      #they should be removed from the stream for normal use
      # Do we add these to a separate list?
      hook_event_count += 1
      fin.read(event.event_size-20)
    else:
      #unknown chunk, skip, don't add to event list
      unknown_event_count += 1
      fin.read(event.event_size-20)
    
    s = fin.read( s_header.size )
    
    prog.update_amount(fin.tell())
    
  print "\n"
  print "events that wil be tracked: %d"%len(event_list)
  print "events total in log: %d"%count
  print "events unknown: %d"%unknown_event_count
  if hook_event_count:
    print "events hook: %d"%hook_event_count
    print "WARNING: event log contains 'hook' events used for debugging and validation. These should not be output by the runtime under normal usage\n"
  fin.close()
        
  return event_list

def do_preprocess_simulation(heap, pre_event_list, event_list):
  prog = ProgressBar(0, len(pre_event_list) + len(event_list), 77, mode='fixed', char='#', autoprint=True)
  frees_and_reallocs = {}
  preprocessed_memory = 0

  for event in event_list:
    if event.event_type == Event.eRealloc:
      frees_and_reallocs[ event.pDataPrev ] = 0
    elif event.event_type == Event.eFree:
      frees_and_reallocs[ event.pData ] = 0
    prog.increment_amount()

  for event in pre_event_list:
    if event.event_type == event.eAlloc:
      if frees_and_reallocs.has_key(event.pData):
        heap.track( event )
        preprocessed_memory += event.allocSize
    elif event.event_type == Event.eRealloc:
      if frees_and_reallocs.has_key(event.pData):
        heap.track( event )
        preprocessed_memory += event.allocSize
    prog.increment_amount()
  print "\n\npreprocessed_memory = %s"%locale.format('%d', preprocessed_memory, True)

def find_marker( event_list, marker_id ):
  index = 0;
  for event in event_list:
    if event.event_type == event.eMarker and event.marker_type == marker_id:
      return index
    index += 1
  return -1

def prune_event_list( event_list, opts ):
  begin_index = 0
  if opts.begin_marker >= 0:
    begin_index = find_marker(event_list, opts.begin_marker)
    if begin_index == -1:
      begin_index = 0
    else:
      print "  Found beginrt marker %d"%opts.begin_marker

  end_index = len(event_list)
  if opts.end_marker >= 0:
    end_index = find_marker(event_list, opts.end_marker)
    if end_index == -1:
      end_index = len(event_list)
    else:
      print "  Found end marker %d"%opts.end_marker

  return event_list[begin_index:end_index]

def do_simple_processing(pre_event_list, event_list):
  allocated = 0
  reallocated = 0
  reallocated_freed = 0
  freed = 0
  num_events = 0
  blocks = {}

  print "Preprocessing (looking for frees and reallocs)..."
  prog = ProgressBar(0, len(event_list), 77, mode='fixed', char='#', autoprint=True)
  # find all free's and realloc's.
  # the logic is that there will be a lot less free's and realloc's than alloc's
  # and so we only track the addresses which will be free'd or realloc'd instead
  # of all of the addresses which are alloc'd
  # NOTE: we only care about free's and realloc's within our begin/end markers
  for event in event_list:
    if event.event_type == Event.eRealloc:
      blocks[ event.pDataPrev ] = 0
    elif event.event_type == Event.eFree:
      blocks[ event.pData ] = 0
    prog.increment_amount()

  print "\n\nSimulating events..."
  prog = ProgressBar(0, len(pre_event_list) + len(event_list), 77, mode='fixed', char='#', autoprint=True)
  for event in pre_event_list:
    if event.event_type == event.eAlloc:
      if blocks.has_key(event.pData):
        blocks[ event.pData ] = event.allocSize
    elif event.event_type == Event.eRealloc:
      if blocks.has_key(event.pData):
        blocks[ event.pData ] = event.allocSize
    prog.increment_amount()

  for event in event_list:
    num_events += 1
    if event.event_type == event.eAlloc:
      allocated += event.allocSize
      if blocks.has_key(event.pData):
        blocks[ event.pData ] = event.allocSize
    elif event.event_type == Event.eRealloc:
      reallocated += event.allocSize
      reallocated_freed += blocks[ event.pDataPrev ]
      blocks[event.pData] = 0
      if blocks.has_key(event.pData):
        blocks[ event.pData ] = event.allocSize
    elif event.event_type == Event.eFree:
      freed += blocks[ event.pData ]
      blocks[event.pData] = 0
    prog.increment_amount()

  total = allocated + reallocated - reallocated_freed - freed

  print "\n\n"
  print "allocated =         %15s"%locale.format('%d', allocated, True)
  print "reallocated =       %15s"%locale.format('%d', reallocated, True)
  print "reallocated_freed = %15s"%locale.format('%d', reallocated_freed, True)
  print "freed =             %15s"%locale.format('%d', freed, True)
  print "-----------------------------------"
  print "total =             %15s"%locale.format('%d', total, True)
  print "\n"
  print "total events simulated  = %s"%locale.format('%d', num_events, True)

def main():
  # define command line arguments/parser
  parser = optparse.OptionParser('%%prog %s' % __version__)
  parser.add_option("-i", "--input", help="Input log file path", metavar="FILE")
  parser.add_option("-d", "--dict", help="Input dictionary file path", metavar="FILE")
  parser.add_option("-a", "--address_db", help="Input address symbols sql db file path", metavar="FILE")
  parser.add_option("-x", "--histogram", help="Report histogram", metavar="ENABLE", action="store_true", default=True)
  parser.add_option("-X", "--nohistogram", help="DO NOT report histogram", metavar="ENABLE", action="store_false", dest="histogram")
  parser.add_option("-c", "--callhier", help="Report call hierarchy", metavar="ENABLE", action="store_true", default=True)
  parser.add_option("-C", "--nocallhier", help="Do NOT Report call hierarchy", metavar="ENABLE", action="store_false", dest="callhier")
  parser.add_option("-e", "--errors", help="Report errors", metavar="ENABLE", action="store_true", default=True)
  parser.add_option("-E", "--noerrors", help="Do NOT Report errors", metavar="ENABLE", action="store_false", dest="errors")
  parser.add_option("-y", "--callstack_detail", help="Write callstack detail to report", metavar="ENABLE", action="store_true", default=True)
  parser.add_option("-Y", "--no_callstack_detail", help="Do NOT Report callstack detail", metavar="ENABLE", action="store_false", dest="callstack_detail")

  parser.add_option("-p", "--prune_callstacks", help="Prune callstacks above memory manager functions", metavar="ENABLE", action="store_true", default=True)
  parser.add_option("-P", "--no_prune_callstacks", help="Do NOT prune callstacks", metavar="ENABLE", action="store_false", dest="prune_callstacks")

  parser.add_option("-l", "--limit", help="size limit for reporting information", type="int", default=0x10000)
  parser.add_option("", "--limit_event_reads", help="allocation limit for reporting branch", type="int", default=0x7FFFFFF)
  parser.add_option("", "--call_stack_write_limit", help="limit number of callstacks written to report (development feature)", type="int", default=-1)
  
  parser.add_option("", "--begin_marker", help="Marker ID to start simulation", type="int", default=-1)
  parser.add_option("", "--end_marker", help="Marker ID to end simulation", type="int", default=-1)

  parser.add_option("-S", "--simple_processing", help="Do simple total allocated/freed analysis", metavar="ENABLE", action="store_true", default=False)

  parser.add_option("-v", "--verbose", help="Do verbose logging", metavar="ENABLE", action="store_true")
  opts, args = parser.parse_args()

  in_name     = 'memtrack_log.dat'
  in_dict     = 'memtrack_dict.dat'
  in_address  = 'address.db'
  
  ##########################
  # apply command line arbuments
#  pprint.pprint( opts )
  verbose = opts.verbose
  if opts.input:
    in_name = opts.input
  if opts.dict:
    in_dict = opts.dict
  if opts.address_db:
    in_address = opts.address_db
    
  if not os.path.exists( in_name ):
    print>>sys.stderr, 'Could not find input file: %s' % in_name
    sys.exit(128)
 
  if not os.path.exists( in_dict ):
    print>>sys.stderr, 'Could not find dictionary: %s' % in_dict
    sys.exit(128)

  if not os.path.exists( in_address ):
    print>>sys.stderr, 'Could not find symbol db: %s' % in_address
    sys.exit(128)

  MemTrack.event_log_read_limit = opts.limit_event_reads
  MemTrack.reporting_size_limit = opts.limit
  MemTrack.prune_callstacks = opts.prune_callstacks
  MemTrack.call_stack_write_limit = opts.call_stack_write_limit
  
  ##########################
  print "Loading...\n\n"
  
  ### cache the address symbol db in a dict for ease of use
  print "Caching symbols...\n"
  SymbolDB.symbol_cache = build_address_symbol_cache( in_address )
  print "\n\n"

  ### Read in the call stack dictionary
  print "Reading callstack dictionary...\n"
  stack_dict = read_callstack_dictionary( in_dict )
  print "\n\n"
 
  ### Read the event log
  print "Reading event log..."
  event_list = read_event_log( in_name )
  print "\n\n"


  ##########################
  pre_event_list = []

  if opts.begin_marker >= 0 or opts.end_marker >= 0:
    print "Pruning list...\n\n"
    begin_index = 0
    if opts.begin_marker >= 0:
      begin_index = find_marker(event_list, opts.begin_marker)
      if begin_index == -1:
        begin_index = 0
      else:
        print "  Found begin marker: %d at index %d"%(opts.begin_marker, begin_index)

    end_index = len(event_list)
    if opts.end_marker >= 0:
      end_index = find_marker(event_list, opts.end_marker)
      if end_index == -1:
        end_index = len(event_list)
      else:
        print "  Found end marker  : %d at index %d"%(opts.end_marker, end_index)

    if begin_index > 0:
      pre_event_list = event_list[:begin_index]

    event_list = event_list[begin_index:end_index]

    print "\n  Pruned events that will be tracked: %d\n"%len(event_list)
    gc.collect()

  if len(event_list) == 0:
    return

  if opts.simple_processing:
    do_simple_processing(pre_event_list, event_list)
    return

  ##########################
  print "Processing...\n\n"

  lpath, ltail = os.path.split( in_name )
  lname, lext = os.path.splitext( ltail )
  outName = lname.split(' ')[0]
  outName = outName.split('_')[0]
  
  ### histogram
  if opts.histogram:
    print "<histogram feature not available w/o html and graph generation.>\n\n"
    print "\n\n"
  
  ### heap simulation
  #create a heap to track live allocated blocks and current memory status overall
  heap = Heap()
  if len(pre_event_list) > 0:
    print "Preprocessing events...\n\n"
    do_preprocess_simulation(heap, pre_event_list, event_list)
    print "\n\n"

  print "Simulating events...\n\n"
  prog = ProgressBar(0, len(event_list), 77, mode='fixed', char='#', autoprint=True)
  for event in event_list:
    heap.track( event )
    prog.increment_amount()
  print "\n"
  if len(heap.errors()):
    print "simulation errors: %d of %d"%(len(heap.errors()), len(event_list))
  else:
    print "simulation errors: NONE"
  print "\n\n"
  
  ### call hierarchy
  if opts.callhier:
    print "Generating call hierarchy allocation tree...\n\n"
    hierarchy_root = generate_hierarchy( event_list, stack_dict )
    print "entry points: %d"%len(hierarchy_root.children)

##########################
  print "complete."

if __name__=="__main__":
    main()



