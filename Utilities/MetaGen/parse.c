/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "timer.h"
#include "fileio.h"
#include "parse.h"
#include "ncHash.h"
#include "Array.h"
#include "Str.h"

#define TYPE_T           Arg
#define TYPE_FUNC_PREFIX aarg
#include "Array_def.c"

#define TYPE_T           Parse
#define TYPE_FUNC_PREFIX aparse
#include "Array_def.c"

static const char *GenType_strs[] = {
    "",         //     GENTYPE_ERROR,      
    "char",     //     GENTYPE_CHAR,      
    "short",    //     GENTYPE_SHORT,     
    "int",      //     GENTYPE_INT,       
    "long",     //     GENTYPE_LONG,      
    "unsigned", //     GENTYPE_UNSIGNED,  
    "float",    //     GENTYPE_FLOAT,     
    "double",   //     GENTYPE_DOUBLE,    
    "void",     //     GENTYPE_VOID,      
    "S8",       //     GENTYPE_S8,        
    "U8",       //     GENTYPE_U8,        
    "S16",      //     GENTYPE_S16,       
    "U16",      //     GENTYPE_U16,       
    "S32",      //     GENTYPE_S32,       
    "U32",      //     GENTYPE_U32,       
    "S64",      //     GENTYPE_S64,       
    "U64",      //     GENTYPE_U64,       
    "F32",      //     GENTYPE_F32,
};

static GenType GenType_from_str(char *str)
{
    int i;
    for( i = 0; i < ARRAY_SIZE(GenType_strs); ++i ) 
    {
        if(0 == strcmp(GenType_strs[i],str))
            return i;
    }
    return GENTYPE_UNKNOWN;
}

ParseRun* parserun_create_dbg(DBG_PARMS_SOLO)
{
    ParseRun *r = MALLOCP(r);
    DBG_PARMS_MBR_ASSIGN(r);
    return r;
}

void parserun_destroy(ParseRun *r)
{
    parserun_cleanup(r);
    free(r);
}

BOOL parserun_init(ParseRun *pr, char *fn)
{
    if(!pr || !fn)
        return FALSE;
    aparse_destroy(&pr->funcs,parse_cleanup);
    pr->lex_ctxt = NULL;
    pr->lex_line_start = NULL;
    ClearStruct(&pr->loc);
    if(fn)
        return file_alloc(&pr->input,fn) != NULL;
    return TRUE;
}

void parse_addfunc(ParseRun *run, char **callnames[METATOKEN_COUNT], Arg *return_type, char *func_name, Arg *args)
{
	int i;
	Parse *p = aparse_push(&run->funcs);
	if(return_type && func_name)
	{
		p->type = ParseType_Func;
		p->ret_type = *return_type;
		strcpy(p->name, func_name);
		p->args = args;
		strcpy(p->filename, run->fn);
		p->fileline = run->loc.first_line;
	}
	else
	{
		p->type = ParseType_Void;
	}

	for(i = 0; i < METATOKEN_COUNT; i++)
	{
		int j;
		for(j = 0; j < ap_size(&callnames[i]); j++)
		{
			char *callname = callnames[i][j];
			if(callname)
				ap_push(&p->callnames[i], callname);
			else if(p->type != ParseType_Void)
				ap_push(&p->callnames[i], Str_dup(p->name));
		}
		ap_destroy(&callnames[i], NULL);
	}
}

void parserun_cleanup(ParseRun *pr)
{
    aparse_destroy(&pr->funcs,parse_cleanup);
    achr_destroy(&pr->input);
    pr->lex_ctxt = NULL;
    pr->lex_line_start = NULL;
    ClearStruct(&pr->loc);
}

void parse_cleanup(Parse *p)
{
    aarg_destroy(&p->args);
}

int yyparse (ParseRun *run);

BOOL parse_metafn_from_fn(char *dst, size_t n, char *nodir, int n_nodir, const char *fn, char *ext)
{
    char fn_nodir[MAX_PATH];
    char *t;

    if(!dir_nofilename(dst,n,fn))
        return FALSE;
    if(!filename_nodir(ASTR(fn_nodir),fn))
        return FALSE;
    t = strrchr(fn_nodir,'.');
    if(!t)
        WARN("couldn't find file extension in fn %s",fn);
    else
        *t = 0; // remove extension

    // a/b/c/foo.c => a/b/c/meta/foo_meta.c
    strcat(fn_nodir,"_meta.");
    strcat(fn_nodir,ext);
    
    strcat_safe(dst,n,"meta/");
    strcat_safe(dst,n,fn_nodir);
    if(nodir)
        strcpy_safe(nodir,n_nodir,fn_nodir);
    return TRUE;
}

static void s_Str_catarg(char **str, Arg *arg)
{
	int i;
	if(arg->is_const)
		Str_catf(str, "const ");
	Str_catf(str, "%s ", arg->type_name);
	for(i = 0; i < arg->n_indirections; i++)
		Str_catf(str, "*");
	Str_catf(str, "%s", arg->name);
}

static void s_Str_catparse_decl(char **str, Parse *p)
{
	int i;
	Str_catf(str, "void %s(", p->name);
	for(i = 0; i < aarg_size(&p->args); i++)
	{
		if(i)
			Str_catf(str, ", ");
		s_Str_catarg(str, &p->args[i]);
	}
	Str_catf(str, ")");
}

static void s_Str_catparse_callargs(char **str, Parse *p)
{
	int i;
	for(i = 0; i < aarg_size(&p->args); i++)
	{
		if(i)
			Str_catf(str, ", ");
		Str_catf(str, "%s", p->args[i].name);
	}
}

static void s_Str_catparse_call(char **str, Parse *p)
{
	Str_catf(str, "%s(", p->name);
	s_Str_catparse_callargs(str, p);
	Str_catf(str, ")");
}

void strupper(char *str)
{
	for(; *str; str++)
		*str = ch_toupper(*str);
}

void strlower(char *str)
{
	for(; *str; str++)
		*str = ch_tolower(*str);
}

// returns non-zero on success
static int filef_write(U8 *data, int len, FORMAT filename, ...)
{
	char path[MAX_PATH];
	FILE *fout;
	errno_t err;
	int wrote;

	VA_START(va, filename);
	vsprintf_s(ASTR(path), filename, va);
	VA_END();

	err = fopen_safe(&fout, path, "w");
	if(err || !fout)
	{
		WARN("couldn't open %s for writing", path);
		return 0;
	}

	wrote = fwrite(data, 1, len, fout);
	if(wrote != len)
	{
		WARN("couldn't not write to %s (%d of %d bytes)", path, wrote, len);
		fclose(fout);
		return 0;
	}

	fclose(fout);
	return 1;
}

BOOL parserun_close_dbg(ParseRun *run, const char *project DBG_PARMS)
{
	char curtime[128];
	char project_uc[MAX_PATH], project_lc[MAX_PATH];
	int i;
	char *buf = Str_temp();
	BOOL ret = TRUE;
	int has_tokentype[METATOKEN_COUNT] = {0};
	int has_internalonlys = 0;
#if 1
	char dir[MAX_PATH];
	sprintf(dir, "Common/%s/", project);
#else
	const char *dir = "MetaGen/";
#endif

	if(!aparse_size(&run->funcs)) // nothing to write
		return TRUE;

	strcpy(project_uc, project);
	strupper(project_uc);
	strcpy(project_lc, project);
	strlower(project_lc);

	if(!ncmkdirtree(dir))
		return FALSE;

	for(i = 0; i < aparse_size(&run->funcs); i++)
	{
		Parse *p = &run->funcs[i];

		int j;
		for(j = 0; j < METATOKEN_COUNT; j++)
		{
			if(ap_size(&p->callnames[j]))
				has_tokentype[j] = 1;

			if(	!ap_size(&p->callnames[METATOKEN_REMOTE]) &&
				!ap_size(&p->callnames[METATOKEN_CONSOLE]) )
			{
				p->internal_only = 1;
				has_internalonlys = 1;
			}
		}
	}

#define printline(fmt, ...) Str_catf(&buf, fmt "\n", __VA_ARGS__)
#if 1 // leave this in until MetaGen becomes part of our build process
#	define printhead() printline(											\
	"// ============================================================\n"		\
	"// META generated file. do not edit!\n"								\
	"// Generated: %s\n"													\
	"// ============================================================\n",	\
	curtime_str(ASTR(curtime))); // FIXME: this should be local time with a time zone
#else
#	define printhead() printline(												\
	"// ============================================================\n"			\
	"// META generated file. do not edit! \144onotcheckin!\n" /* '\144'=='d' */	\
	"// Generated: %s\n"														\
	"// ============================================================\n",		\
	curtime_str(ASTR(curtime))); // FIXME: this should be local time with a time zone
#endif

	// write out the .h
	Str_clear(&buf);
	printhead();
	printline("#pragma once");
	printline("//#include \"ncstd.h\"");
	printline("#include \"UtilsNew/meta.h\"");
	printline("");
	printline("void %s_meta_register(void);", project_lc);
	if(has_tokentype[METATOKEN_REMOTE])
	{
		printline("");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(p->type != ParseType_Void && ap_size(&p->callnames[METATOKEN_REMOTE]))
			{
				s_Str_catparse_decl(&buf, &run->funcs[i]);
				Str_catf(&buf, ";\n");
				p->printed_header = 1;
			}
		}
		printline("");
		printline("// Use these to force a local call while handling a remote call");
		printline("// e.g. If you need to call a function marked with REMOTE from a remote call,");
		printline("//      but you don't want its output to be sent to the original caller");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(p->type != ParseType_Void && ap_size(&p->callnames[METATOKEN_REMOTE]))
			{
				Str_catf(&buf, "#define %s_local(", p->name);
				s_Str_catparse_callargs(&buf, p);
				Str_catf(&buf, ")	meta_local(%s(", p->name);
				s_Str_catparse_callargs(&buf, p);
				Str_catf(&buf, "))\n");
			}
		}
		printline("");
		printline("// Use these to treat the call as a remote call (for output etc.)");
		printline("// These calls are made with an accesslevel of 11 (internal)");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(p->type != ParseType_Void && ap_size(&p->callnames[METATOKEN_REMOTE]))
			{
				Str_catf(&buf, "#define %s_remote(link, forward_id, ", p->name);
				s_Str_catparse_callargs(&buf, p);
				Str_catf(&buf, ")	meta_remote(link, forward_id, %s(", p->name);
				s_Str_catparse_callargs(&buf, p);
				Str_catf(&buf, "))\n");
			}
		}
	}
	if(!filef_write(buf, Str_len(&buf), "%s%s_meta.h", dir, project))
		ret = FALSE;

	// write out the .c
	Str_clear(&buf);
	printhead();
	printline("#include \"%s_meta.h\"", project_lc);
	printline("");
	printline("#include \"netio.h\"");
	printline("");
	printline("// FIXME: thread-safety");
	printline("");
	printline("#include \"UtilsNew/Array.h\"");
	printline("#include \"UtilsNew/lock.h\"");
	if(has_internalonlys)
	{
		printline("");
		printline("#if PROJECT_%s", project_uc);
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(	p->type != ParseType_Void && !p->printed_header &&
				p->internal_only )
			{
				s_Str_catparse_decl(&buf, &run->funcs[i]);
				Str_catf(&buf, ";\n");
				p->printed_header = 1;
			}
		}
		printline("#endif // PROJECT_%s", project_uc);
	}
	if(	has_tokentype[METATOKEN_REMOTE] ||
		has_tokentype[METATOKEN_CONSOLE] )
	{
		printline("");
		printline("#if !CLIENT");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(p->type != ParseType_Void && !p->printed_header)
			{
				s_Str_catparse_decl(&buf, &run->funcs[i]);
				Str_catf(&buf, ";\n");
				p->printed_header = 1;
			}
		}
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(	ap_size(&p->callnames[METATOKEN_REMOTE]) ||
				ap_size(&p->callnames[METATOKEN_CONSOLE]) )
			{
				int j;
				printline("");
				printline("void %s_receivepacket(Packet *pak_in)", p->name);
				printline("{");
				for(j = 0; j < aarg_size(&p->args); j++)
				{
					Arg *a = &p->args[j];
					GenType type = GenType_from_str(a->type_name);
					if(type == GENTYPE_CHAR && a->n_indirections == 1) // FIXME: detection is sloppy
						printline("	char *%s = pktGetStringTemp(pak_in);", a->name);
					else if(type == GENTYPE_INT)
						printline("	int %s = pktGetBitsAuto(pak_in);", a->name);
					else
						assertm("type %s not supported!", a->type_name);
				}
				Str_catf(&buf, "	"); s_Str_catparse_call(&buf, p); Str_catf(&buf, ";\n");
				printline("}");
			}
		}
		printline("#endif // !CLIENT");
	}
	printline("");
	printline("void %s_meta_register(void)", project_lc);
	printline("{");
	printline("	static int registered = 0;");
	printline("	static LazyLock lock = {0};");
	printline("");
	printline("	ObjectInfo *object = NULL;");
	printline("	TypeParameter *parameter = NULL;");
	printline("");
	printline("	if(registered)");
	printline("		return;");
	printline("");
	printline("	lazyLock(&lock);");
	printline("	if(registered)");
	printline("	{");
	printline("		lazyUnlock(&lock);");
	printline("		return;");
	printline("	}");
	for(i = 0; i < aparse_size(&run->funcs); i++)
	{
		int j;
		Parse *p = &run->funcs[i];

		printline("");
		if(p->internal_only)
			printline("#if PROJECT_%s", project_uc);
		else
			printline("#if !CLIENT"); // currently, the client only needs remote-call-forwarders, defined below
		if(p->type == ParseType_Void)
		{
			printline("	object = NULL;");
		}
		else
		{
			printline("	object = calloc(1, sizeof(*object));");
			printline("	object->name = \"%s\";", p->name);
			printline("	object->typeinfo.type = TYPE_FUNCTION;");
			if(ap_size(&p->callnames[METATOKEN_ACCESSLEVEL]))
				printline("	object->accesslevel = %s;", ap_top(&p->callnames[METATOKEN_ACCESSLEVEL]));
			if(ap_size(&p->callnames[METATOKEN_CONSOLE_DESC]))
				printline("	object->console_desc = \"%s\";", ap_top(&p->callnames[METATOKEN_CONSOLE_DESC]));
			if(ap_size(&p->callnames[METATOKEN_CONSOLE_REQPARAMS]))
				printline("	object->console_reqparams = %s;", ap_top(&p->callnames[METATOKEN_CONSOLE_REQPARAMS]));
			else
				printline("	object->console_reqparams = %d;", aarg_size(&p->args));
			printline("	object->location = %s;", p->name);
			printline("	object->filename = \"%s\";", p->filename);
			printline("	object->fileline = %d;", p->fileline);
			if(	ap_size(&p->callnames[METATOKEN_REMOTE]) ||
				ap_size(&p->callnames[METATOKEN_CONSOLE]) )
			{
				printline("	object->receivepacket = %s_receivepacket;", p->name);
			}

			for(j = 0; j < aarg_size(&p->args); j++)
			{
				Arg *a = &p->args[j];
				GenType type = GenType_from_str(a->type_name);

				printline("");
				printline("	parameter = as_push(&object->typeinfo.function_parameters);");
				printline("	parameter->name = \"%s\";", a->name);
				if(type == GENTYPE_CHAR && a->n_indirections == 1)
				{
					// FIXME: detection is sloppy
					printline("	parameter->typeinfo.type = TYPE_STRING;");
				}
				else if(type == GENTYPE_INT)
				{
					printline("	parameter->typeinfo.type = TYPE_INTEGER;");
				}
				else
				{
					assertm("type %s not supported!", a->type_name);
				}
			}
		}
		printline("");
		if(ap_size(&p->callnames[METATOKEN_COMMANDLINE]))
		{
			printline("	#if PROJECT_%s", project_uc);
			for(j = 0; j < ap_size(&p->callnames[METATOKEN_COMMANDLINE]); j++)
				printline("		meta_registerCall(CALLTYPE_COMMANDLINE, object, \"%s\");", p->callnames[METATOKEN_COMMANDLINE][j]);
			printline("	#endif");
		}
		if(ap_size(&p->callnames[METATOKEN_CONSOLE]))
		{
			printline("	#if SERVER");
			for(j = 0; j < ap_size(&p->callnames[METATOKEN_CONSOLE]); j++)
			{
				char *callname = p->callnames[METATOKEN_CONSOLE][j];
				printline("		meta_registerCall(CALLTYPE_CONSOLE, object, \"%s\");", callname);
				if(!strBegins(callname, project))
					printline("		meta_registerCall(CALLTYPE_CONSOLE, object, \"%s_%s\");", project_lc, callname);
			}
			printline("	#endif");
		}
		if(	ap_size(&p->callnames[METATOKEN_REMOTE]) ||
			ap_size(&p->callnames[METATOKEN_CONSOLE]) ) // console implies remote handlers above mapserver
		{
			if(ap_size(&p->callnames[METATOKEN_REMOTE]))
				printline("	#if !CLIENT");
			else
				printline("	#if DBSERVER || PROJECT_%s", project_uc);
			printline("		meta_registerCall(CALLTYPE_REMOTE, object, \"%s\");", p->name);
			for(j = 0; j < ap_size(&p->callnames[METATOKEN_REMOTE]); j++)
				if(strcmp(p->name, p->callnames[METATOKEN_REMOTE][j]) != 0) // FIXME: case sensitivity
					printline("		meta_registerCall(CALLTYPE_REMOTE, object, \"%s\");", p->callnames[METATOKEN_REMOTE][j]);
			printline("	#endif");
		}
		if(p->internal_only)
			printline("#endif // PROJECT_%s", project_uc);
		else
			printline("#endif // !CLIENT");
	}
	printline("");
	printline("	registered = 1;");
	printline("	lazyUnlock(&lock);");
	printline("}");

	// dbserver versions
	if(	has_tokentype[METATOKEN_REMOTE] ||
		has_tokentype[METATOKEN_CONSOLE] )
	{
		printline("");
		printline("#if DBSERVER");
		printline("#include \"comm_backend.h\"");
		printline("#include \"%scomm.h\"", project_lc);
		printline("");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(	ap_size(&p->callnames[METATOKEN_REMOTE]) ||
				ap_size(&p->callnames[METATOKEN_CONSOLE]) )
			{
				int j;
				s_Str_catparse_decl(&buf, p); Str_catf(&buf, "\n");
				printline("{");
				printline("	// put a breakpoint in %s around line %d", p->filename, p->fileline);
				printline("	NetLink *link = %s_getLink();", project_lc);
				printline("	if(link)");
				printline("	{");
				printline("		Packet *pak_out = pktCreateEx(link, %s_CLIENT_COMMAND);", project_uc);
				printline("		meta_pktSendRoute(pak_out);");
				printline("		pktSendStringAligned(pak_out, \"%s\");", p->name);
				for(j = 0; j < aarg_size(&p->args); j++)
				{
					Arg *a = &p->args[j];
					GenType type = GenType_from_str(a->type_name);
					if(type == GENTYPE_CHAR && a->n_indirections == 1) // FIXME: detection is sloppy
						printline("		pktSendStringAligned(pak_out, %s);", a->name);
					else if(type == GENTYPE_INT)
						printline("		pktSendBitsAuto(pak_out, %s);", a->name);
					else
						assertm("type %s not supported!", a->type_name);
				}
				printline("		pktSend(&pak_out, link);");
				printline("	}");
				printline("	else");
				printline("	{");
				printline("		// reply with failure");
				printline("		meta_printf(\"%sNoConnection\");", project);
				printline("	}");
				printline("}");
				printline("");
			}
		}
		printline("#endif // DBSERVER");
	}

	// mapserver versions
	if(	has_tokentype[METATOKEN_REMOTE] ||
		has_tokentype[METATOKEN_CONSOLE] )
	{
		printline("");
		printline("#if SERVER");
		printline("#include \"comm_backend.h\"");
		printline("#include \"dbcomm.h\"");
		printline("");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(	ap_size(&p->callnames[METATOKEN_REMOTE]) ||
				ap_size(&p->callnames[METATOKEN_CONSOLE]) )
			{
				int j;
				s_Str_catparse_decl(&buf, p); Str_catf(&buf, "\n");
				printline("{");
				printline("	// put a breakpoint in %s around line %d", p->filename, p->fileline);
				printline("	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_%s_COMMAND);", project_uc);
				printline("	meta_pktSendRoute(pak_out);");
				printline("	pktSendStringAligned(pak_out, \"%s\");", p->name);
				for(j = 0; j < aarg_size(&p->args); j++)
				{
					Arg *a = &p->args[j];
					GenType type = GenType_from_str(a->type_name);
					if(type == GENTYPE_CHAR && a->n_indirections == 1) // FIXME: detection is sloppy
						printline("	pktSendStringAligned(pak_out, %s);", a->name);
					else if(type == GENTYPE_INT)
						printline("	pktSendBitsAuto(pak_out, %s);", a->name);
					else
						assertm("type %s not supported!", a->type_name);
				}
				printline("	pktSend(&pak_out, &db_comm_link);");
				printline("}");
				printline("");
			}
		}
		printline("#endif // SERVER");
	}

	// game versions
	if(has_tokentype[METATOKEN_REMOTE])
	{
		printline("");
		printline("#if CLIENT");
		printline("#include \"clientcomm.h\"");
		printline("#include \"entVarUpdate.h\"");
		printline("");
		for(i = 0; i < aparse_size(&run->funcs); i++)
		{
			Parse *p = &run->funcs[i];
			if(ap_size(&p->callnames[METATOKEN_REMOTE]))
			{
				int j;
				s_Str_catparse_decl(&buf, p); Str_catf(&buf, "\n");
				printline("{");
				printline("	// put a breakpoint in %s around line %d", p->filename, p->fileline);
				printline("	START_INPUT_PACKET(pak_out, CLIENTINP_%s_COMMAND);", project_uc);
				printline("	meta_pktSendRoute(pak_out);");
				printline("	pktSendStringAligned(pak_out, \"%s\");", p->name);
				for(j = 0; j < aarg_size(&p->args); j++)
				{
					Arg *a = &p->args[j];
					GenType type = GenType_from_str(a->type_name);
					if(type == GENTYPE_CHAR && a->n_indirections == 1) // FIXME: detection is sloppy
						printline("	pktSendStringAligned(pak_out, %s);", a->name);
					else if(type == GENTYPE_INT)
						printline("	pktSendBitsAuto(pak_out, %s);", a->name);
					else
						assertm("type %s not supported!", a->type_name);
				}
				printline("	END_INPUT_PACKET;");
				printline("}");
				printline("");
			}
		}
		printline("#endif // CLIENT");
	}

	if(!filef_write(buf, Str_len(&buf), "%s%s_meta.c", dir, project))
		ret = FALSE;

	Str_destroy(&buf);
	return ret;
}

BOOL parserun_processfile_dbg(ParseRun *run, char *fn DBG_PARMS)
{
	if(!fn)
	{
		WARN("no filename given");
		return FALSE;
	}
	filename_nodir(ASTR(run->fn), fn);

	file_alloc_dbg(&run->input, fn DBG_PARMS_CALL);
	if(!Str_len(&run->input))
	{
		WARN("couldn't open file %s",fn);
		return FALSE;
	}

	if(!strstr(run->input, "META("))
		return TRUE;

	run->lex_ctxt = NULL;
	run->lex_line_start = NULL;
	if(0!=yyparse(run))
	{
		WARN("couldn't parse %s",fn);
		return FALSE;
	}

	return TRUE;
}
