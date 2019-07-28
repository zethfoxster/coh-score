import datetime
from fractions import Fraction
import os
import re
import sys
import pprint
  
INPUT = ['systemspecs.log']
DATE_RANGES = [('4-01-2011', '5-15-2011')]

USER_LIST = None #'C:/systemspecs/betausers-na.txt'
FILE_FILTER = re.compile('') # Use 'USCO' or 'DECO' to filter North America or EU.

OUTPUT = 'ss.csv'

INPUT_FOLDER = 'C:\cmd\system_specs_to_process'

#INPUT = [
#	'C:\dev\python\SystemSpecs.log',
#	'C:/SystemSpecs/Freedom/SystemSpecs.txt',
#	'C:/SystemSpecs/Freedom/SystemSpecs_1.txt',
#	'C:/SystemSpecs/Freedom/SystemSpecs_2.txt',
#	'C:/SystemSpecs/Freedom/SystemSpecs_4.txt'
#    'c:/systemspecsabbr.log'
#    'c:/SystemSpecs/ss_champion.log',
#    'c:/SystemSpecs/ss_freedom.log',
#    'c:/SystemSpecs/ss_guardian.log',
#    'c:/SystemSpecs/ss_infinity.log',
#    'c:/SystemSpecs/ss_justice.log',
#    'c:/SystemSpecs/ss_liberty.log',
#    'c:/SystemSpecs/ss_pinnacle.log',
#    'c:/SystemSpecs/ss_protector.log',
#    'c:/SystemSpecs/ss_triumph.log',
#    'c:/SystemSpecs/ss_victory.log',
#    'c:/SystemSpecs/ss_virtue.log',
#]

class TrackedField:
	def __str__(self):
		return 'id=%s, title=%s, min_percent=%f'%(self.id, self.title, self.min_percent)

	def __init__(self, id, title, min_percent = 0.0 ):
		self.id = id
		self.title = title
		self.min_percent = min_percent

		
FIELD_INITS = [
	('Operating System', 'Operating System', 0.0 ),
	('CPU Cores', 'CPU Cores', 0.0 ),
	('System RAM', 'System RAM', 0.0 ),
	('Launcher', 'Launcher', 0.0 ),
	('Adapter Driver Date',	'Adapter Driver Date', 0.0 ), # currently this is of the primary adapter, which may not be the same as gl context uses
	('OpenGL Version', 'OpenGL Version', 0.0 ),
	('GPU Manufacturer', 'GPU Manufacturer', 0.0 ),
	('GPU Chipset Family', 'GPU Chipset Family', 0.0 ),
	('Resolution', 'Resolution', 0.02 ),
	('Render Path', 'Render Path', 0.0 ),
	('GPU-mac', 'GPU Detail Mac', 0.0 ),
	('xGPU', 'GPU Detail', 0.0 ),
	
# some of these can be useful for more detail or testing
	# 'xGPU',
	# 'GPU-mac',
	# 'GL_RENDERER',

    # 'SSE Support'
	# ' VideoCardVendorID',
	# ' VideoCardDeviceID',
    # ' DriverDate',
    # 'Widescreen',
]

# use initializer tuples to build up list of tracked field spec objects
FIELDS = [TrackedField(*args) for args in FIELD_INITS]
[pprint.pprint(str(field)) for field in FIELDS]

INF_FILES = [
#    'n:/users/MCHOCK/9.2-CX_75974.inf',
#    'n:/users/MCHOCK/9.4-CX_78234.inf',
#    'n:/users/MCHOCK/nvdm.inf',
#    'n:/users/MCHOCK/nv4_disp.inf',
]

# @todo
# add year released and distinguish mobile parts
GPU_CHIPSET_FAMILIES = [
	[re.compile("geforce.*\D5[567][0]"), "GeForce 500", "NVIDIA"],
	[re.compile("geforce.*\D4[45678][05]"), "GeForce 400", "NVIDIA"],
	[re.compile("geforce.*2\d[05]"), "GeForce 200", "NVIDIA"],
	[re.compile("geforce.*\D3\d\d\D"), "GeForce 200", "NVIDIA"],
	[re.compile("geforce.*9\d\d0"), "GeForce 9", "NVIDIA"],
	[re.compile("quadro.*\D1\d\dm"), "GeForce 9", "NVIDIA"],
	[re.compile("geforce.*\D1\d\dm"), "GeForce 9", "NVIDIA"],
	[re.compile("geforce ion"), "GeForce 9", "NVIDIA"],
	[re.compile("geforce.*8\d\d0"), "GeForce 8", "NVIDIA"],
	[re.compile("quadro.*\D1600m"), "GeForce 8", "NVIDIA"],
	[re.compile("quadro.*\D570"), "GeForce 8", "NVIDIA"],
	[re.compile("quadro.*\D140m"), "GeForce 8", "NVIDIA"],
	[re.compile("quadro.*\D135m"), "GeForce 8", "NVIDIA"],
	[re.compile("geforce.*gt.*1[2-3]0"), "GeForce 8", "NVIDIA"],
	[re.compile("geforce.*g100"), "GeForce 8", "NVIDIA"],
	[re.compile("nforce 750a"), "GeForce 8", "NVIDIA"],
	[re.compile("mcp67m"), "GeForce 7", "NVIDIA"],
	[re.compile("geforce.*7\d\d[05]"), "GeForce 7", "NVIDIA"],
	[re.compile("quadro.*\D1[1-2]\dm"), "GeForce 7", "NVIDIA"],
	[re.compile("quadro.*\D[1-4]50"), "GeForce 7", "NVIDIA"],
	[re.compile("gladiac.*\D7\d\d\D"), "GeForce 7", "NVIDIA"],
	[re.compile("geforce.*6\d\d0"), "GeForce 6", "NVIDIA"],
	[re.compile("quadro fx.*[1-4]400"), "GeForce 6", "NVIDIA"],
	[re.compile("geforce.*5\d\d0"), "GeForce 5 (FX)", "NVIDIA"],
	[re.compile("geforce.*fx"), "GeForce 5 (FX)", "NVIDIA"],
	[re.compile("geforce4.*mx"), "GeForce 4 MX", "NVIDIA"],
	[re.compile("geforce4.*\D4\d\d\D"), "GeForce 4 MX", "NVIDIA"],
	[re.compile("geforce4.*4000"), "GeForce 4 MX", "NVIDIA"],
	[re.compile("geforce4.*ti"), "GeForce 4 Ti", "NVIDIA"],
	[re.compile("geforce4.*4[1-9]\d\d"), "GeForce 4 Ti", "NVIDIA"],
	[re.compile("geforce3 ti"), "GeForce 3 Ti", "NVIDIA"],
	[re.compile("geforce3"), "GeForce 3", "NVIDIA"],
	[re.compile("geforce2"), "GeForce 2", "NVIDIA"],

	[re.compile("radeon.*6\d\d\d"), "Radeon HD 6xxx", "AMD"],
	[re.compile("ati mobility firegl.*5\d\d\d"), "R600", "AMD"],
	[re.compile("radeon.*5\d\d\d"), "Radeon HD 5xxx", "AMD"],
	[re.compile("radeon.*4\d\d\d"), "R700", "AMD"],
	[re.compile("radeon.*[2-3]\d\d\d"), "R600", "AMD"],
	[re.compile("amd 760g"), "R600", "AMD"],
	[re.compile("radeon.*x1[3-9]\d\d"), "R520", "AMD"],
	[re.compile("diamond.*x1[3-9]\d\d"), "R520", "AMD"],
	[re.compile("radeon.*x[7-8]\d0"), "R420", "AMD"],
	[re.compile("radeon.*xpress.*12[0-7]\d"), "R420", "AMD"],
	[re.compile("radeon.*x12\d\d"), "R400", "AMD"],
	[re.compile("radeon.*x1050"), "R300", "AMD"],
	[re.compile("radeon.*x[3-6]\d0"), "R300", "AMD"],
	[re.compile("radeon.*9[5-8]\d\d"), "R300", "AMD"],
	[re.compile("radeon.*9[0-2]\d0"), "R200", "AMD"],
	[re.compile("radeon.*8[5-9]00"), "R200", "AMD"],
	[re.compile("radeon.*7\d00"), "R100", "AMD"],
	[re.compile("radeon.*igp.*\D3[24][05]\D"), "R100", "AMD"],

	[re.compile("intel.*gma x3100"), "GMA X3100", "Intel"],
	[re.compile("intel.*graphics media accelerator hd"), "GMA HD", "Intel"],
	[re.compile("intel.*hd graphics"), "GMA HD", "Intel"],
	[re.compile("mobile intel.*965\D"), "GMA X3100", "Intel"],
	[re.compile("intel.*4 series"), "GMA X4500", "Intel"],
	[re.compile("intel.*\D45 express"), "GMA X4500", "Intel"],
	[re.compile("intel.*g4[5]\D"), "GMA X4500HD", "Intel"],
	[re.compile("intel.*g4[13]\D"), "GMA X4500", "Intel"],
	[re.compile("intel.*q4[35]\D"), "GMA 4500", "Intel"],
	[re.compile("intel.*g35\D"), "GMA X3500", "Intel"],
	[re.compile("intel.*965\D"), "GMA X3100", "Intel"],
	[re.compile("intel.*g965\D"), "GMA X3000", "Intel"],
	[re.compile("intel.*q3[35]\D"), "GMA 3100", "Intel"],
	[re.compile("intel.*g3[13]\D"), "GMA 3100", "Intel"],
	[re.compile("intel.*graphics media accelerator 3150"), "GMA 3100", "Intel"],
	[re.compile("intel.*q96\d\D"), "GMA 3000", "Intel"],
	[re.compile("intel.*946gz\D"), "GMA 3000", "Intel"],
	[re.compile("intel.*945\D"), "GMA 950", "Intel"],
	[re.compile("intel.*91\d\D"), "GMA 900", "Intel"],
	[re.compile("intel.*graphics media accelerator.*500\D"), "GMA 500", "Intel"],
	[re.compile("intel.*828\d\d"), "Extreme Graphics", "Intel"],
	[re.compile("intel.*extreme graphics"), "Extreme Graphics", "Intel"],
	
	[re.compile("transgaming"), "Unknown Mac (Cider)", "Unknown"],
	[re.compile("parallels"), "Unknown Mac (Parallels)", "Unknown"],
	[re.compile("x ?11"), "Unknown X11", "Unknown"],
#   [re.compile("nvidia"), "Unknown NVIDIA", "NVIDIA"],
#    [re.compile("ati"), "Unknown AMD", "AMD"],
#    [re.compile("radeon"), "Unknown RADEON", "AMD"],
#    [re.compile("intel"), "Unknown Intel", "Intel"],
#	[re.compile(""), "Unrecognized", "Unknown"],
]

GPU_STR_REPLACEMENTS = [
	("winfast px", "geforce "),
	("asus a", "radeon "),
	("asus eah", "radeon "),
	("all-in-wonder", "radeon"),
	("diamond s", "radeon "),
	("xtasy", "radeon "),
	("xpress 200", "x300"),
	("xpress 1100", "x300"),
	("xpress 1150", "x300"),
	("nvidia", "geforce")
]

lastGpuLookup = (None, None)

WINDOWS_VERSIONS = {
	(4, 0): 'Windows 95 or NT 4.0',
	(4, 10): 'Windows 98',
	(4, 90): 'Windows ME',
	(5, 0): 'Windows 2000',
	(5, 1): 'Windows XP',
	(5, 2): 'Windows XP x64 Professional',
	(6, 0): 'Windows Vista',
	(6, 1): 'Windows 7'
}

class FieldNotFound(Exception):
	pass

infData = {}

def getVideoInf(path):
	input = open(path)
	NONE = 0
	DATA = 1
	STRINGS = 2
	state = NONE
	for line in input:
		line = line.strip()
		if len(line) == 0 or line[0] == ';':
			pass
		elif line == '[ATI.Mfg.NTx86]':
			manufacturer = 'AMD'
			state = DATA
		elif line == '[NVIDIA.Mfg]':
			manufacturer = 'NVIDIA'
			state = DATA
		elif line == '[Strings]':
			state = STRINGS
		elif line[0] == '[':
			state = NONE
		elif state == DATA:
			lineParts = line.split('=')
			card = lineParts[0].strip()
			remainder = lineParts[1].strip()
			gpu = remainder.split(',')[0]
			infData[card] = (gpu, manufacturer)
		elif state == STRINGS:
			lineParts = line.split('=')
			src = '%' + lineParts[0].strip() + '%'
			if src in infData:
				infData[lineParts[1].strip()] = infData[src]
				del infData[src]

def lookupGpu(videoCard):
	global lastGpuLookup
	if videoCard == lastGpuLookup[0]:
		return lastGpuLookup[1]
	if videoCard in infData:
		data = infData[videoCard]
		lastGpuLookup = (videoCard, [None, data[0], data[1]])
		return lastGpuLookup[1]
	name = videoCard.lower()
#	print videoCard
	
	for r in GPU_STR_REPLACEMENTS:
		name = name.replace(r[0], r[1])
	for family in GPU_CHIPSET_FAMILIES:
		re = family[0]
		if (re.search(name)):
			lastGpuLookup = (videoCard, family)
			return family
	print videoCard
	lastGpuLookup = (videoCard, None)
	return None

def getField(entry, fieldName, isContinuation=False, offset=1):
	global lastVideoCard
	global lastGpuChipset
	if fieldName == 'CPU Cores':
		return getField(entry, ' NumRealCPUs')
	if fieldName == 'System RAM':
#        return getField(entry, ' Memory')
		ram = int(getField(entry, ' Memory'))
		RAM_LEVELS = [512, 1024, 2048, 4096]
		for level in RAM_LEVELS:
			if ram < level:
				return '< ' + str(level) + ' MB'
		return '>= ' + str(RAM_LEVELS[-1]) + ' MB'
	if fieldName == 'GPU Chipset Family':
		# we use the GL_RENDERER field now to determine the GPU family as the VideoCard spec is for the primary
		# display adapter which may not be what the GL context is running on (e.g. Optimus systems)
		gpu_string_orig = getField(entry, ' GL_RENDERER')
		gpu_string = gpu_string_orig.lower()
#		gpu_string = gpu_string.replace( '"', "" )
		# truncate at first slash which usally has PCI details
#		gpu_string = gpu_string.partition(r'/')[0]
		gpuInfo = lookupGpu(gpu_string)
		if (gpuInfo == None):
			print gpu_string
			return "Unknown" # (" + gpu_string_orig + ")"
		return gpuInfo[1]
	if fieldName == 'Primary Adapter Manufacturer':
		# this is not necessarily the adapter the GL context is on
		# also probably simpler and more accurate to just use the PCI VideoCardVendorID field
		videoCard = getField(entry, ' VideoCard')
		gpuInfo = lookupGpu(videoCard)
		if (gpuInfo == None):
			return "Unknown (" + videoCard + ")"
		return gpuInfo[2]

	if fieldName == 'Adapter Driver Date':
		# currently this is of the primary adapter, which may not be the same as gl context uses
		date = getField(entry, ' DriverDate')
		date = date.strip('"')
		if date != "Unknown":
			date = date[-4:] # just get the year
		return date

	if fieldName == 'Resolution':
		width = getField(entry, ' ScreenX')
		height = getField(entry, ' ScreenY')
		if getField(entry, ' Fullscreen') == '1':
			return width + 'x' + height
		pixels = int(width) * int(height)
		if pixels < 500000:
			return 'Windowed (< 0.5 MPixels)'
		if pixels < 750000:
			return 'Windowed (< 0.75 MPixels)'
		if pixels < 800000:
			return 'Windowed (< 0.8 MPixels)'
		if pixels < 1000000:
			return 'Windowed (< 1.0 MPixels)'
		if pixels < 1250000:
			return 'Windowed (< 1.25 MPixels)'
		if pixels < 1500000:
			return 'Windowed (< 1.5 MPixels)'
		if pixels < 2000000:
			return 'Windowed (< 2.0 MPixels)'
		return 'Windowed (>= 2.0 MPixels)'
	if fieldName == 'Video Driver':
		return getField(entry, 'GPU Manufacturer') + ' v' + getField(entry, ' DriverVersion').strip('"')
	if fieldName == 'SSE Support':
		# CPUIDs can contain commas; use the last field result as a key to add to append to the string
		cpu = getField(entry, ' CPUIdentifier', True).strip('"')
		cpuFields = cpu.split(' ')
		manufacturer = cpuFields[-1]
		familyModel = (int(getField(cpuFields, 'Family')),
			int(getField(cpuFields, 'Model')))
		if manufacturer == 'GenuineIntel':
			if familyModel in [(6, 26)]:
				return 'SSE4.2'
			if familyModel in [(6, 23)]:
				return 'SSE4.1'
			if familyModel in [(6, 8)]:
				return 'SSE'
		return cpu
	if fieldName == 'Render Path':
		return getField(entry, ' RenderPath')
	if fieldName == 'CPU Cores':
		return getField(entry, ' NumCPUs')
	if fieldName == 'Operating System':
		videoCard = getField(entry, ' VideoCard').lower()
		if videoCard.find('transgaming') != -1:
			return 'OS X (Cider)'
		if videoCard.find('x 11') != -1:
			return 'Unknown OS running X11'
		winVersion = (int(getField(entry, 'OSVersion')), 
					  int(getField(entry, 'OSVersion', offset=2)))
		if winVersion in WINDOWS_VERSIONS:
			result = WINDOWS_VERSIONS[winVersion]
		else:
			result = 'Unrecognized Windows version ' + str(winVersion)
		if videoCard.find('parallels') != -1:
			result = result + ' (under Parallels)'
		return result
	if fieldName == 'OSVersion':
		try:
			return getField(entry, ' OSVersion', isContinuation, offset)
		except:
			pass
	if fieldName == 'Widescreen':
		width = int(getField(entry, ' ScreenX'))
		height = int(getField(entry, ' ScreenY'))
		aspect = Fraction(width, height)
		result = 'Yes'
		if aspect < Fraction(160, 100):
			result = 'No'
 		if getField(entry, ' Fullscreen') == '1':
 			result += ' (fullscreen)'
 		else:
 			result += ' (windowed)'
 		return result
	if fieldName == 'Launcher':
		using_launcher = getField(entry, ' Launcher')
		if using_launcher == '1':
			result = "yes"
		else:
			result = "no"
		return result
	if fieldName == 'GPU Detail':
		return getField(entry, 'xGPU')
	if fieldName == 'GL_RENDERER':
		return getField(entry, ' GL_RENDERER')
	if fieldName == 'GPU Manufacturer':
		# we use the GL_VENDOR now to determine the GPU manufacturer as the VideoCard spec is for the primary
		# display adapter which may not be what the GL context is running on (e.g. Optimus systems)
		gl_vendor = getField(entry, ' GL_VENDOR')
		#coalesce some common company name variations
		gl_vendor = gl_vendor.strip('"')
		return gl_vendor.split(' ')[0].strip()
	if fieldName == 'OpenGL Version':
		gl_version = getField(entry, ' GL_VERSION')
		re_gl_version = re.compile(r'^"([0-9]+)[.]([0-9]+).*')
		match = re_gl_version.match(gl_version)
		if match:
#			pprint.pprint(match.groups())
			gl_version_digits = match.group(1) + '.' + match.group(2)
#			print gl_version_digits
			return gl_version_digits
		else:
			return "unknown"
	if fieldName == 'xGPU':
		gpu_string = getField(entry, ' GL_RENDERER')
		gpu_string = gpu_string.lower()
		gpu_string = gpu_string.replace( '"', "" )
		# truncate at first slash which usally has PCI details
		gpu_string = gpu_string.partition(r'/')[0]
		gpu_string = gpu_string.replace( "nvidia", "" )
		gpu_string = gpu_string.replace( "asus", "" )
		gpu_string = gpu_string.replace( "sapphire", "" )
		gpu_string = gpu_string.replace( "diamond", "" )
		gpu_string = gpu_string.replace( "visiontek", "" )
		gpu_string = gpu_string.replace( "opengl", "" )
		gpu_string = gpu_string.replace( "engine", "" )
		gpu_string = gpu_string.replace( "series", "" )
		gpu_string = gpu_string.replace( "ati", "" )
		gpu_string = gpu_string.replace( "amd", "" )
		gpu_string = gpu_string.replace( "mobility", "" )
		gpu_string = gpu_string.replace( "graphics", "" )
		gpu_string = gpu_string.replace( " pro", "" )
		gpu_string = gpu_string.replace( " xt", "" )
		gpu_string = gpu_string.replace( " gtx", "" )
		gpu_string = gpu_string.replace( " gts", "" )
		gpu_string = gpu_string.replace( " gt", "" )
		gpu_string = gpu_string.replace( " gs", "" )
		gpu_string = gpu_string.replace( " agp", "" )
		gpu_string = gpu_string.replace( "ultra", "" )
		gpu_string = gpu_string.replace( "x86", "" )
		gpu_string = gpu_string.replace( "(r)", "" )
		gpu_string = gpu_string.replace( "(tm)", "" )
		gpu_string = gpu_string.replace( "pci", "" )
		gpu_string = gpu_string.replace( "ddr", "" )
		gpu_string = gpu_string.replace( "sse2", "" )
		gpu_string = gpu_string.replace( "sse", "" )
		gpu_string = gpu_string.replace( "(microsoft corporon - wddm)", "" )
		
		pieces = gpu_string.split(' ')
		gpu_string = ' '.join(pieces)
		gpu_string = gpu_string.strip()
		return gpu_string
	if fieldName == 'GPU-mac' or fieldName == 'GPU Detail Mac':
		os = getField(entry, 'Operating System').lower()
		if os.startswith("os x"):
			return getField(entry, 'xGPU')
		return '<<discard>>'		# so we throw away non mac cards
		
	try:
		index = entry.index(fieldName) + offset
	except ValueError:
		raise FieldNotFound()
	return entry[index]
	result = entry[index]
	if isContinuation and result[-1] != '"':
		result = result + ',' + getField(entry, result, True)
	return result;

dateRangeFormat = re.compile(r'(\d+)-(\d+)-(\d+).*')
def getDateRangeDate(line):
	match = dateRangeFormat.match(line)
	return datetime.date(int(match.group(3)), int(match.group(1)), int(match.group(2)))

dateFormat = re.compile(r'(\d\d)(\d\d)(\d\d).*')
def getDate(line):
	match = dateFormat.match(line)
	return datetime.date(int('20'+match.group(1)), int(match.group(2)), int(match.group(3)))

# compact a single log file for date range and authname
def compactLog(filename):
	dateRanges = [(getDateRangeDate(dateRange[0]), getDateRangeDate(dateRange[1])) for dateRange in DATE_RANGES]
	resultDict = {}
	users = None
	if USER_LIST:
		users = frozenset([line.strip() for line in open(USER_LIST)])
	linesRead = 0
	print(filename)
	sys.stdout.flush()
	for line in open(filename):
#		print line
		linesRead = linesRead + 1
		date = getDate(line)
#		print date
		inRange = [(date >= dateRange[0] and date < dateRange[1]) for
			dateRange in dateRanges]
		if any(inRange):
			entry = line.split(',')
			authname = getField(entry, 'AuthName')
			if authname and (users == None or authname in users):
				for i in range(len(dateRanges)):
					if inRange[i]:
								resultDict[(authname, i)] = entry
	return [(items[0][1], items[1]) for items in resultDict.items()]

#scan all the logs in the given folder by date range and unique auth
def compactLogs(folder_name):
	dateRanges = [(getDateRangeDate(dateRange[0]), getDateRangeDate(dateRange[1])) for dateRange in DATE_RANGES]
	resultDict = {}
	users = None
	if USER_LIST:
		users = frozenset([line.strip() for line in open(USER_LIST)])
	linesRead = 0
	for dirpath, dirnames, filenames in os.walk(folder_name):
		for filename in filenames:
			file = os.path.join(dirpath, filename)
			if not FILE_FILTER.search(file):
				print('Skipping ' + file)
				continue
			else:
#				print(file)
				sys.stdout.flush()
				for line in open(file):
			#		print line
					linesRead = linesRead + 1
					date = getDate(line)
			#		print date
					inRange = [(date >= dateRange[0] and date < dateRange[1]) for
						dateRange in dateRanges]
					if any(inRange):
						entry = line.split(',')
						authname = getField(entry, 'AuthName')
						if authname and (users == None or authname in users):
							for i in range(len(dateRanges)):
								if inRange[i]:
									resultDict[(authname, i)] = entry
	return [(items[0][1], items[1]) for items in resultDict.items()]

def recordField(results, entry, field, interval):
	if field.id not in results:
		results[field.id] = {}
	try:
		fieldValue = getField(entry, field.id)
	except FieldNotFound:
		fieldValue = 'Not reported'
	if fieldValue != '<<discard>>':	# gives us a way to throw out results, e.g. tack just mac GPUs
		if fieldValue not in results[field.id]:
			results[field.id][fieldValue] = [0] * len(DATE_RANGES)
		lastValue = results[field.id][fieldValue]
		results[field.id][fieldValue][interval] = results[field.id][fieldValue][interval] + 1

def asPercent(num, denom):
	return 0.001 * (100000 * num / denom)

def csvEscape(s):
	return '"' + s.replace('"', '""') + '"'

def reportField(out, results, field, totalEntries):
	fieldResults = results[field]
	keys = [key for key in fieldResults.keys()]
	keys.sort()
	out.write(field + '\n')
	for key in keys:
		if len(DATE_RANGES) == 2:
			pct0 = asPercent(fieldResults[key][0], totalEntries[0])
			pct1 = asPercent(fieldResults[key][1], totalEntries[1])
			out.write(csvEscape(str(key)) + ',' + str(pct0) + '%,' + str(pct1) + '%,' + str(pct1-pct0) + '%\n')
		else:
			out.write(csvEscape(str(key)) + ',')
			for i in range(len(DATE_RANGES)):
				pct = asPercent(fieldResults[key][i], totalEntries[i])
				out.write(str(pct) + '%,')
			out.write('\n')
	out.write('\n')

# make a simple list of sorted (label, count) enumerations for a given date range index for easier reporting
# if min_frac is greater than zero than any entries with less than that fraction of total items will
# be lumped into an 'other' data row. useful for pie charts, etc.
def generate_field_data_rows( results, field_name, date_range_index = 0, min_frac = 0.0 ):
	fieldResults = results[field_name]

	values_sorted = sorted(fieldResults.items(), key=lambda (k,v): v[date_range_index], reverse=True )

	data_rows = []
	
	#count the actual entries in the results as we may have discarded some for
	#reporting on subsets of the total (e.g. just GPUs on Mac OS X)
	total = 0
	for item in values_sorted:
		count = item[1][date_range_index]
		total += count

	included_count = 0		# the count of items we add to data rows for figuring out what we put in 'other'
		
	# for key in keys:
		# fraction = (fieldResults[key][date_range_index])/float(total)
	for item in values_sorted:
		count = item[1][date_range_index]
		fraction = count/float(total)
		if fraction >= min_frac:
			data_rows.append( (item[0],count) )
			included_count += count

	remainder = total - included_count;
	if remainder > 0:
		data_rows.append( ('other',remainder) )

	return (data_rows,total)
	
def generate_field_table_html( title, id, rows, total ):
	table_id = 'table_' + id
	html  = '  <table class="sorting" id="%s">\n'%table_id
	header_template_first = '   <th class="first" onclick="%s">%s<span class="sortarrow"></span></th>\n'
	header_template = '   <th onclick="%s">%s<span class="sortarrow"></span></th>\n'
	handler_template = "SortTable('%s',%d,'%s');return false;"
	
	html += '  <tr>'
	html += header_template_first%( handler_template%(table_id, 0, 'alpha'), title )
	html += header_template%( handler_template%(table_id, 1, 'num'), "count" )
	html += header_template%( handler_template%(table_id, 2, 'num'), "percent" )
	html += '  </tr>\n'

	html += '  <tbody>\n'

	for row in rows:
		count = row[1]
		fraction = count/float(total)
		percent = fraction * 100.0
		html += '  <tr>'
		html += '   <td>%s</td>'%row[0]
		html += '   <td class="n">%s</td>'%str(count)
		html += '   <td class="n">% 5.2f</td>'%percent
		html += '  </tr>\n'
		
	html += '  </tbody></table>\n'
	html += "<p></p>\n"

	return html

def generate_field_report_html( title, id, rows, total ):
	#use div and css sytles with positioning to make two column layout
	html = '<div id="container">\n'
	
	html += ' <div id="div-table">\n'
	html += generate_field_table_html( title, id, rows, total )
	html += ' </div>\n'
	
	html += ' <div id="div-graph">\n'
	html += '  <div id="%s"></div>\n'%id
#	html += '  <div id="%s" style="width: 400px; height: 100px;"></div>\n'%id
	html += ' </div>\n'
	
	html += ' <div id="clear"/>\n'	#reset flow
#	html += ' <br/><span/>\n'

	html += '</div>\n'	# end container

	return html

	
def generate_chart_js( title, id, rows ):
	js_template = r'''
		<script type="text/javascript">
			function chart_<id>()
			{
				// Create and populate the data table.
				var data = new google.visualization.DataTable();
				data.addColumn('string', '<title>');
				data.addColumn('number', '# of users');

				<data_rows>
				
				// Create and draw the visualization.
				pc = new google.visualization.PieChart(document.getElementById('<id>'));
				var literalOptions = {width: 450, height: 250, is3D: true, title: '<title>', pieSliceText:'percent',
				pieSliceTextStyle:{color: 'black', fontName: 'Arial', fontSize: 10} };

				pc.draw(data, literalOptions );
			}		
			google.setOnLoadCallback(chart_<id>);
		</script>
	'''
	js = js_template.replace( '<id>', id )
	js = js.replace('<title>', title)
	
	#generate lines to populate the data source
	num_rows = len(rows)
	data_source_str = 'data.addRows(%s);\n'%str(num_rows)
	for i,row in enumerate(rows):
		row1 = "data.setValue(%s,0,'%s');\n"%(str(i),row[0])
		row2 = "data.setValue(%s,1,%s);\n"%(str(i),str(row[1]))
		data_source_str += row1 + row2
	js = js.replace('<data_rows>', data_source_str)
	return js
	
def generate_html_javascripts(resutls, totalEntries):
	html = ""
	for field in FIELDS:
		print 'generating graphs for "%s"'%field.title
		rows, total = generate_field_data_rows( results, field.id, min_frac=field.min_percent )
		id = field.id.replace(' ', '_')
		html += generate_chart_js( field.title, id, rows )
	return html
	
def generate_html_body(resutls, totalEntries):
	html = ""
	for field in FIELDS:
		print 'generating tables for "%s"'%field.title
		rows, total = generate_field_data_rows( results, field.id, min_frac=field.min_percent )
		id = field.id.replace(' ', '_')
		html += generate_field_report_html( field.title, id, rows, total )
	return html
	
def generate_html(results, totalEntries):
	head = r'''
	<?xml version="1.0" encoding="UTF-8"?>
	<! DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml">
	<head>
	<style type="text/css">
		table
		{
		  font-size: medium;
		}
		th
		{
		  padding-right: 0.45cm;
		  padding-left: 0.45cm;
		  text-align: center
		}
		table.sorting th
		{
		  padding-right: 0.25em;
		  padding-left: 0.25em;
		  font-weight: bolder;
		  cursor: pointer;
		  color: #404040;
		  background-color: gainsboro;
		  text-decoration: none;
		  min-width: 50px;
		}
		table.sorting th.first
		{
		  min-width: 175px;
		}
		table.sorting th.i
		{
		  cursor:default;
		  color:#FFFFFF;
		  background-color:#FFFFFF;
		  text-decoration:none
		}
		td
		{
		  padding-right: 0.1em;
		  padding-left: 0.1em;
		  color: black;
		  background-color: #f5f5f5
		}
		td.n
		{
		  padding-right: 0.25em;
		  text-align: right
		}
		#clear {
			clear:both;
			background-color:#fff;
			color:#fff;
		}
		#container {
			border-top:groove;
			position:relative;
			background-color:#fff;
			color:#fff;
		}
		#div-table {
			padding-top: 1.0em;
			background-color:#fff;
			color:#fff;
			width: 350px;
			float: left;
		}
		#div-graph {
			background-color:#fff;
			color:#fff;
			width: 350px;
			float: left;
		}
	</style>
	<script type="text/javascript">
		var g=-1;var h=0;var j;function text(c){if(typeof c=="string")return c;if(c.innerText)return c.innerText;if(c.textContent)return c.textContent;return"";}function lexical(a,b){var k=a.getElementsByTagName('td');var l=b.getElementsByTagName('td');var m=text(k[g]).toLowerCase();var n=text(l[g]).toLowerCase();if(m==n)return 0;if(m<n)return-1;return 1;}function numeric(a,b){var k=a.getElementsByTagName('td');var l=b.getElementsByTagName('td');var m=text(l[g]).replace(/,/g,'');var n=text(k[g]).replace(/,/g,''); return m-n;}function SortTable(d,e,f){if(j!=d){g=-1;h=false;j=d;}var o=document.getElementById(d);var p=(g==e);var q=o.getElementsByTagName('tbody')[1];var r=q.rows;var s=new Array();for(var i=0;i<r.length;i++){s[i]=r[i].cloneNode(true);}if(p)s.reverse();else{g=e;if(f=="num")s.sort(numeric);else s.sort(lexical);}var t=document.createElement('tbody');for(var i=0;i<s.length;i++){t.appendChild(s[i]);}o.replaceChild(t,q);var u=o.getElementsByTagName('tbody')[0];var v=u.getElementsByTagName('span');for(var i=0;i<v.length;i++){if(i==e){if(p&&!h){h=1;v[i].innerHTML='&uArr;';}else{h=0;v[i].innerHTML='&dArr;';}}else{v[i].innerHTML='';}}}function ttog(g){g.parentNode.className=(g.parentNode.className=="op")?"cl":"op";return false;}
		function utog(g){g.className=(g.className=="op")?"cl":"op";return false;}			
	</script>
    <script type="text/javascript" src="http://www.google.com/jsapi"></script>
    <script type="text/javascript">
      google.load('visualization', '1', {packages: ['corechart']});
    </script>
	%s
	<title> %s </title>
	</head>'''
	
	date_range_index = 0	# first date range
	date_range = DATE_RANGES[date_range_index]
	date_range_label = dateRange[0] + ' to ' + dateRange[1]
	title = "Player Configuration Statistics: %s"%date_range_label
	
	js = generate_html_javascripts(results, totalEntries)
	html = head % (js, title)
	
	html += '<body>\n'	# begin body
	html +="<h2>%s</h2>\n"%title
	html += '<p>sample size: %s</p>\n'%str(totalEntries[0])
	html += generate_html_body(results, totalEntries)
	html += '</body>\n'	# end body
		
	return html
	

if __name__ == '__main__':
#	compactedLog = compactLog(INPUT[0])	# single file processing, testing
	compactedLog = compactLogs(INPUT_FOLDER)	# scan the entire folder
	if not compactedLog or not len(compactedLog):
		print "No valid log entries\n"
		exit()
	else:
		print '%d unique logins' % len(compactedLog)
#   for file in INF_FILES:
#       getVideoInf(file)
	results = {}
	totalEntries = [0] * len(DATE_RANGES)
	for (interval, entry) in compactedLog:
		for field in FIELDS:
			recordField(results, entry, field, interval)
		totalEntries[interval] = totalEntries[interval] + 1
	out = open(OUTPUT, 'w')
	if len(DATE_RANGES) == 2:
		out.write(',' + DATE_RANGES[0][0] + ' to ' + DATE_RANGES[0][1] +
			',' + DATE_RANGES[1][0] + ' to ' + DATE_RANGES[1][1] + ',Change\n')
	else:
		for dateRange in DATE_RANGES:
			out.write(',' + dateRange[0] + ' to ' + dateRange[1])
		out.write('\n')
	# for field in FIELDS:
		# reportField(out, results, field, totalEntries)
	# out.close()
		
	#html report
	html = generate_html( results, totalEntries )
	
	fout = open("ss.html", 'w')
	fout.write(html)
	fout.close()
	