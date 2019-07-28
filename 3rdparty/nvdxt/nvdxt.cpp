/****************************************************************************************
	
    Copyright (C) NVIDIA Corporation 2003

    TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
    *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
    BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
    WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
    BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
    ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
    BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*****************************************************************************************/




#include "nvDXT.h"




HFILE filein = 0;
HFILE fileout;









char * supported_extensions[] =
{
    "*.psd",
    "*.png",
    "*.tga",
    "*.bmp",
    "*.gif",
    "*.ppm",
    "*.jpg",
    "*.jpeg",
    "*.tif",
    "*.tiff",
    "*.cel",
    "*.dds",
    0,
};









char * PopupDirectoryRequestor(const char * initialdir, const char * title)
{
    static char temp[MAX_PATH_NAME];
    static char dir[MAX_PATH_NAME];
    
    BROWSEINFO bi;
    LPITEMIDLIST list;
    
    //HINSTANCE hInstance = GetModuleHandle(NULL);
    //gd3dApp->Create( hInstance  );
    
    bi.hwndOwner = 0; 
    bi.pidlRoot = 0; 
    bi.pszDisplayName = dir; 
    bi.lpszTitle = title; 
    bi.ulFlags = 0; 
    bi.lpfn = 0; 
    bi.lParam = 0; 
    bi.iImage = 0; 
    
    list = SHBrowseForFolder(&bi);
    
    if (list == 0)
        return 0;
    
    SHGetPathFromIDList(list, dir);
    
    return dir;
}




HRESULT nvDXT::compress_all( )
{
    
    struct _finddata_t image_file_data;
    long hFile;
    

    std::vector<std::string>  image_files;

    int i = 0;
    int j;
    while(supported_extensions[i])
    {
        std::string fullwildcard = input_dirname;
        fullwildcard += "\\";
        fullwildcard += supported_extensions[i];

        hFile = _findfirst( fullwildcard.c_str(), &image_file_data );
        if (hFile != -1)
        {
            
            image_files.push_back(image_file_data.name);
            //compress_image_file(filetga.name, TextureFormat);
            
            
            // Find the rest of the .image files 
            while( _findnext( hFile, &image_file_data ) == 0 )
            {
                //compress_image_file(filetga.name, TextureFormat);
                image_files.push_back(image_file_data.name);
            }
            
            _findclose( hFile );
        }
        i++;
    }


    // remove .dds if other name exists

    int n = image_files.size();
    int * tags = new int[n];
    for(i=0; i<n; i++)
        tags[i] = 0;

    // for every .dds file, see if there is an existing non dds file
    for(i=0; i<n; i++)
    {

        std::string & filename  = image_files[i];
        int pos = find_string(filename, ".dds");
        // found a .dds file
        if (pos != -1)
        {

            std::string prefix;
            prefix = filename.substr(0, pos);

            // search all files for prefix
            for(j=0; j<n; j++)
            {
                // .. don't match self
                if (i == j)
                    continue;

                // get second file
                int pos2 = find_string(image_files[j], ".dds");

                // if not a .dds file
                if (pos2 == -1)
                {
                    pos2 = find_string(image_files[j], const_cast<char *>(prefix.c_str()));
                    if (pos2 == 0)
                    {
                        tags[i] = 1;
                        break;
                    }

                }
            }
        }
    }


    HRESULT ahr = 0;
    for(i=0; i<n; i++)
    {
        if (tags[i] == 0)
        {
            std::string & filename  = image_files[i];
            HRESULT hr = compress_image_file(const_cast<char *>(filename.c_str()), TextureFormat);
            if (hr)
                ahr = hr; // last error
        }
    }

    delete [] tags;

    return ahr;

}


void nvDXT::compress_recursive(const char * dirname, HRESULT & ahr)
{

                        
    input_dirname = dirname;

    printf("Processing %s\n", dirname);
    HRESULT hr = ProcessDirectory();
    if (hr)
        ahr = hr;

    
    std::string fullname = dirname;
    fullname += "\\*.*";




    //char *wildcard = "*.*";
    
    struct _finddata_t image_file_data;
    long hFile;
    
    
    std::vector<std::string>  image_files;
    
    
    hFile = _findfirst( fullname.c_str(), &image_file_data );
    
    if (hFile != -1 && image_file_data.attrib & _A_SUBDIR)
    {
        if (image_file_data.name[0] != '.')
        {
            std::string subname = dirname;
            subname += "\\";
            subname += image_file_data.name; 

            compress_recursive(subname.c_str(), ahr);
        }
    }



    
    
    if (hFile != -1)
    {
        
        image_files.push_back(image_file_data.name);
        //compress_image_file(filetga.name, TextureFormat);
        
        
        // Find the rest of the .image files 
        while( _findnext( hFile, &image_file_data ) == 0 )
        {
            //compress_image_file(filetga.name, TextureFormat);
            image_files.push_back(image_file_data.name);
            
            if (image_file_data.attrib &_A_SUBDIR)//Subdirectory. Value: 0x10
            {
                if (image_file_data.name[0] != '.')
                {
                    std::string subname = dirname;
                    subname += "\\";
                    subname += image_file_data.name; 
                    
                    compress_recursive(subname.c_str(), ahr);
                }
                
            }
            
            
        }
        
        _findclose( hFile );
    }

}



bool nvDXT::ProcessFileName(char * wildcard, 	std::string &fullwildcard, std::string & dirname)
{
	std::string temp;


	if (strstr(wildcard, ":\\") || wildcard[0] == '\\')
	{
		temp = wildcard;
		int pos = temp.find_last_of("\\", strlen(wildcard));
		if (pos >= 0)
			dirname = temp.substr(0, pos);
		fullwildcard = wildcard;

		input_dirname = dirname;	
        return true;
	}
	else if (wildcard[0] == '//')
	{
		temp = wildcard;
		int pos = temp.find_last_of("//", strlen(wildcard));
		if (pos >= 0)
			dirname = temp.substr(0, pos);
		fullwildcard = wildcard;

		input_dirname = dirname;		
        return true;
	}
	else if (strrchr(wildcard, '\\'))
	{
		temp = wildcard;
		int pos = temp.find_last_of("\\", strlen(wildcard));
		if (pos >= 0)
			dirname = temp.substr(0, pos);

		fullwildcard = input_dirname;
		fullwildcard += "\\";
		fullwildcard += wildcard;


		input_dirname = dirname;
        return true;

	}
	else if (strrchr(wildcard, '//'))
	{
		temp = wildcard;
		int pos = temp.find_last_of("//", strlen(wildcard));
		if (pos >= 0)
			dirname = temp.substr(0, pos);

		fullwildcard = input_dirname;
		fullwildcard += "\\";
		fullwildcard += wildcard;

		input_dirname = dirname;
        return true;
	}
	else
	{
		fullwildcard = input_dirname;
		fullwildcard += "\\";
		fullwildcard += wildcard;
        return false;
	}
    
}

HRESULT nvDXT::compress_some(char * wildcard)
{
    
    
    int i;
    struct _finddata_t image_file_data;
    long hFile;
    

    std::vector<std::string>  image_files;
    
    int j;
    // Find first .c file in current directory 

	std::string fullwildcard;
    std::string dirname;

    ProcessFileName(wildcard, fullwildcard, dirname);
    

    std::string name;
    hFile = _findfirst( fullwildcard.c_str(), &image_file_data );
    if (hFile != -1)
    {
        
		// pre-pend directory to name
		//
		
		name = dirname;
		name += "\\";
		name += image_file_data.name;
        image_files.push_back(name);
        //compress_image_file(filetga.name, TextureFormat);
        
        
        // Find the rest of the .image files 
        while( _findnext( hFile, &image_file_data ) == 0 )
        {
            //compress_image_file(filetga.name, TextureFormat);

			name = dirname;
			name += "\\";
			name += image_file_data.name;

            image_files.push_back(name);
        }
        
        _findclose( hFile );
    }
    
    
    // remove .dds if other name exists

    int n = image_files.size();
    int * tags = new int[n];
    for(i=0; i<n; i++)
        tags[i] = 0;

    // for every .dds file, see if there is an existing non dds file
    for(i=0; i<n; i++)
    {

        std::string & filename  = image_files[i];
        int pos = find_string(filename, ".dds");
        // found a .dds file
        if (pos != -1)
        {

            std::string prefix;
            prefix = filename.substr(0, pos);

            // search all files for prefix
            for(j=0; j<n; j++)
            {
                // .. don't match self
                if (i == j)
                    continue;

                // get second file
                int pos2 = find_string(image_files[j], ".dds");

                // if not a .dds file
                if (pos2 == -1)
                {
                    pos2 = find_string(image_files[j], const_cast<char *>(prefix.c_str()));
                    if (pos2 == 0)
                    {
                        tags[i] = 1;
                        break;
                    }

                }
            }
        }
    }


    HRESULT ahr = 0;

    for(i=0; i<n; i++)
    {
        if (tags[i] == 0)
        {
            std::string & filename  = image_files[i];

            HRESULT hr = compress_image_file(const_cast<char *>(filename.c_str()), TextureFormat);
            if (hr)
                ahr = hr;
        }
    }

    delete [] tags;

    return ahr;


}


HRESULT nvDXT::compress_list( )
{
    
    FILE *fp = fopen( listfile.c_str(), "r");
    
    if (fp == 0)
    {
        fprintf(stdout, "Can't open list file <%s>\n", listfile.c_str());
        return ERROR_BAD_LIST_FILE;
    }
    
    HRESULT ahr = 0;
    char buff[MAX_PATH_NAME];
    while(fgets(buff, MAX_PATH_NAME, fp))
    {      
        // has a crlf at the end
        int t = strlen(buff);
        buff[t - 1] = 0;




        std::string fullname;
        std::string dirname;


        std::string save_input_dirname = input_dirname;


        bFullPathSpecified = ProcessFileName(buff, fullname, dirname);



        HRESULT hr = compress_image_file(const_cast<char *>(fullname.c_str()), TextureFormat);
        if (hr)
            ahr = hr;


        input_dirname = save_input_dirname;

    }

    fclose(fp);

    return ahr;

}
  




void usage()
{
    fprintf(stdout,"NVDXT\n");
    fprintf(stdout,"This program\n");
    fprintf(stdout,"   compresses images\n");
    fprintf(stdout,"   creates normal maps from color or alpha\n");
    fprintf(stdout,"   creates DuDv map\n");
    fprintf(stdout,"   creates cube maps\n");
    fprintf(stdout,"   writes out .dds file\n");
    fprintf(stdout,"   does batch processing\n");
    fprintf(stdout,"   reads .tga, .bmp, .gif, .ppm, .jpg, .tif, .cel, .dds, .png and .psd\n");
    fprintf(stdout,"   filters MIP maps\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"Options:\n");
    fprintf(stdout,"  -profile <profile name> : Read a profile created from the Photoshop plugin\n");
    fprintf(stdout,"  -quick : use fast compression method\n");
    fprintf(stdout,"  -prescale <int> <int>: rescale image to this size first\n");
    fprintf(stdout,"  -rescale <nearest | hi | lo | next_lo>: rescale image to nearest, next highest or next lowest power of two\n");
    fprintf(stdout,"  -rel_scale <float, float> : relative scale of original image. 0.5 is half size Default 1.0, 1.0\n");
    fprintf(stdout,"  -clamp <int, int> : maximum image size. image width and height are clamped\n");

    fprintf(stdout,"  -nomipmap : don't generate MIP maps\n");
    fprintf(stdout,"  -nmips <int> : specify the number of MIP maps to generate\n");

    fprintf(stdout,"  -rgbe : Image is RGBE format\n");
    fprintf(stdout,"  -dither : add dithering\n");
    
    
    
    fprintf(stdout,"  -sharpenMethod <method>: sharpen method MIP maps\n");
    fprintf(stdout,"  <method> is \n");
    fprintf(stdout,"        None\n");
    fprintf(stdout,"        Negative\n");
    fprintf(stdout,"        Lighter\n");
    fprintf(stdout,"        Darker\n");
    fprintf(stdout,"        ContrastMore\n");
    fprintf(stdout,"        ContrastLess\n");
    fprintf(stdout,"        Smoothen\n");
    fprintf(stdout,"        SharpenSoft\n");
    fprintf(stdout,"        SharpenMedium\n");
    fprintf(stdout,"        SharpenStrong\n");
    fprintf(stdout,"        FindEdges\n");
    fprintf(stdout,"        Contour\n");
    fprintf(stdout,"        EdgeDetect\n");
    fprintf(stdout,"        EdgeDetectSoft\n");
    fprintf(stdout,"        Emboss\n");
    fprintf(stdout,"        MeanRemoval\n");
    fprintf(stdout,"        UnSharp <radius, amount, threshold>\n");
    fprintf(stdout,"        XSharpen <xsharpen_strength, xsharpen_threshold>\n");
    //fprintf(stdout,"        WarpSharp\n");
    fprintf(stdout,"        Custom\n");




    fprintf(stdout,"  -volume <filename> : create volume texture <filename>. Files specified with -list option\n");
    fprintf(stdout,"  -flip : flip top to bottom \n");
    fprintf(stdout,"  -timestamp : Update only changed files\n");
    fprintf(stdout,"  -list <filename> : list of files to convert\n");
    fprintf(stdout,"  -cubemap <filename> : create cube map <filename>. Files specified with -list option\n");
    fprintf(stdout,"                        positive x, negative x, positive y, negative y, positive z, negative z\n");
    fprintf(stdout,"  -all : all image files in current directory\n");
    fprintf(stdout,"  -outdir <directory>: output directory\n");
    fprintf(stdout,"  -deep [directory]: include all subdirectories\n");
    fprintf(stdout,"  -outsamedir : output directory same as input\n");
    fprintf(stdout,"  -overwrite : if input is .dds file, overwrite old file\n");
    fprintf(stdout,"  -file <filename> : input file to process. Accepts wild cards\n");
    fprintf(stdout,"  -output <filename> : filename to write to\n");
    fprintf(stdout,"  -append <filename_append> : append this string to output filename\n");
    fprintf(stdout,"  -24 <dxt1c | dxt1a | dxt3 | dxt5 | u1555 | u4444 | u565 | u8888 | u888 | u555> : compress 24 bit images with this format\n");
    fprintf(stdout,"  -32 <dxt1c | dxt1a | dxt3 | dxt5 | u1555 | u4444 | u565 | u8888 | u888 | u555> : compress 32 bit images with this format\n");
    fprintf(stdout,"\n");

    fprintf(stdout,"  -swap : swap rgb\n");
    fprintf(stdout,"  -binaryalpha : treat alpha as 0 or 1\n");
    fprintf(stdout,"  -alpha_threshold <byte>: [0-255] alpha reference value \n");
    fprintf(stdout,"  -alphaborder : border images with alpha = 0\n");
    fprintf(stdout,"  -fadeamount <int>: percentage to fade each MIP level. Default 15\n");

    fprintf(stdout,"  -fadecolor : fade map (color, normal or DuDv) over MIP levels\n");
    fprintf(stdout,"  -fadetocolor <hex color> : color to fade to\n");

    fprintf(stdout,"  -fadealpha : fade alpha over MIP levels\n");
    fprintf(stdout,"  -fadetoalpha <byte>: [0-255] alpha to fade to\n");

    fprintf(stdout,"  -border : border images with color\n");
    fprintf(stdout,"  -bordercolor <hex color> : color for border\n");
    fprintf(stdout,"  -force4 : force DXT1c to use always four colors\n");


    fprintf(stdout,"\n");
    
    

    fprintf(stdout,"Texture Format  Default DXT3:\n");
    fprintf(stdout,"  -dxt1c : DXT1 (color only)\n");
    fprintf(stdout,"  -dxt1a : DXT1 (one bit alpha)\n");
    fprintf(stdout,"  -dxt3  : DXT3\n");
    fprintf(stdout,"  -dxt5  : DXT5\n\n");
    fprintf(stdout,"  -u1555 : uncompressed 1:5:5:5\n");
    fprintf(stdout,"  -u4444 : uncompressed 4:4:4:4\n");
    fprintf(stdout,"  -u565  : uncompressed 5:6:5\n");
    fprintf(stdout,"  -u8888 : uncompressed 8:8:8:8\n");
    fprintf(stdout,"  -u888  : uncompressed 0:8:8:8\n");
    fprintf(stdout,"  -u555  : uncompressed 0:5:5:5\n");
    fprintf(stdout,"  -p8    : paletted 8 bit (256 colors)\n");
    fprintf(stdout,"  -p4    : paletted 4 bit (16 colors)\n");
    fprintf(stdout,"  -a8    : 8 bit alpha channel\n");
    fprintf(stdout,"  -cxv8u8: normal maps\n");
    fprintf(stdout,"  -A8L8   : 8 bit alpha channel, 8 bit luminance\n");
    fprintf(stdout,"  -fp32x4 : fp32 four channels (A32B32G32R32F)\n");
    fprintf(stdout,"  -fp32   : fp32 one channel (R32F)\n");
    fprintf(stdout,"  -fp16x4 : fp16 four channels (A16B16G16R16F)\n");

    fprintf(stdout,"\n");

    fprintf(stdout,"Optional Mip Map Filtering. Default box filter:\n");

    fprintf(stdout,"  -Point\n"); 
    fprintf(stdout,"  -Box\n");
    fprintf(stdout,"  -Triangle\n");
    fprintf(stdout,"  -Quadratic\n");
    fprintf(stdout,"  -Cubic\n");

    fprintf(stdout,"  -Catrom\n");
    fprintf(stdout,"  -Mitchell\n");

    fprintf(stdout,"  -Gaussian\n");
    fprintf(stdout,"  -Sinc\n");
    fprintf(stdout,"  -Bessel\n");

    fprintf(stdout,"  -Hanning\n");
    fprintf(stdout,"  -Hamming\n");
    fprintf(stdout,"  -Blackman\n");
    fprintf(stdout,"  -Kaiser\n");

    fprintf(stdout,"\n");

    
    
    fprintf(stdout,"***************************\n");
    fprintf(stdout,"To make a normal or dudv map, specify one of\n");
    fprintf(stdout,"  -n4 : normal map 4 sample\n");
    fprintf(stdout,"  -n3x3 : normal map 3x3 filter\n");
    fprintf(stdout,"  -n5x5 : normal map 5x5 filter\n");
    fprintf(stdout,"  -n7x7 : normal map 7x7 filter\n");
    fprintf(stdout,"  -n9x9 : normal map 9x9 filter\n");
    fprintf(stdout,"  -dudv : DuDv\n");

    fprintf(stdout,"\n");


    fprintf(stdout,"and source of height info:\n");
    fprintf(stdout,"  -alpha : alpha channel\n");
    fprintf(stdout,"  -rgb : average rgb\n");
    fprintf(stdout,"  -biased : average rgb biased\n");
    fprintf(stdout,"  -red : red channel\n");
    fprintf(stdout,"  -green : green channel\n");
    fprintf(stdout,"  -blue : blue channel\n");
    fprintf(stdout,"  -max : max of (r,g,b)\n");
    fprintf(stdout,"  -colorspace : mix of r,g,b\n");
    fprintf(stdout,"  -norm : normalize mip maps (source is a normal map)\n\n");
    fprintf(stdout,"-signed : signed output for normal maps\n");

    fprintf(stdout,"\n");

    fprintf(stdout,"Normal/DuDv Map dxt:\n");
    fprintf(stdout,"  -aheight : store calculated height in alpha field\n");
    fprintf(stdout,"  -aclear : clear alpha channel\n");
    fprintf(stdout,"  -awhite : set alpha channel = 1.0\n");
    fprintf(stdout,"  -scale <float> : scale of height map. Default 1.0\n");
    fprintf(stdout,"  -wrap : wrap texture around. Default off\n");
    fprintf(stdout,"  -minz <int> : minimum value for up vector [0-255]. Default 0\n\n");


    fprintf(stdout,"***************************\n");
    fprintf(stdout,"To make a depth sprite, specify:");
    fprintf(stdout,"\n");
    fprintf(stdout,"  -depth\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"and source of depth info:\n");
    fprintf(stdout,"  -alpha  : alpha channel\n");
    fprintf(stdout,"  -rgb    : average rgb (default)\n");
    fprintf(stdout,"  -red    : red channel\n");
    fprintf(stdout,"  -green  : green channel\n");
    fprintf(stdout,"  -blue   : blue channel\n");
    fprintf(stdout,"  -max    : max of (r,g,b)\n");
    fprintf(stdout,"  -colorspace : mix of r,g,b\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"Depth Sprite dxt:\n");
    fprintf(stdout,"  -aheight : store calculated depth in alpha channel\n");
    fprintf(stdout,"  -aclear : store 0.0 in alpha channel\n");
    fprintf(stdout,"  -awhite : store 1.0 in alpha channel\n");
    fprintf(stdout,"  -scale <float> : scale of depth sprite (default 1.0)\n");

    
    fprintf(stdout,"\n");
    fprintf(stdout,"\n");
	fprintf(stdout,"Examples\n");
    fprintf(stdout,"  nvdxt -cubemap cubemap.dds -list cubemapfile.lst\n");
    fprintf(stdout,"  nvdxt -file test.tga -dxt1c\n");
    fprintf(stdout,"  nvdxt -file *.tga\n");
	fprintf(stdout,"  nvdxt -file c:\\temp\\*.tga\n");
	fprintf(stdout,"  nvdxt -file temp\\*.tga\n");
    fprintf(stdout,"  nvdxt -file height_field_in_alpha.tga -n3x3 -alpha -scale 10 -wrap\n");
    fprintf(stdout,"  nvdxt -file grey_scale_height_field.tga -n5x5 -rgb -scale 1.3\n");
    fprintf(stdout,"  nvdxt -file normal_map.tga -norm\n");
    fprintf(stdout,"  nvdxt -file image.tga -dudv -fade -fadeamount 10\n");
    fprintf(stdout,"  nvdxt -all -dxt3 -gamma -outdir .\\dds_dir -time\n");
    fprintf(stdout,"  nvdxt -file *.tga -depth -max -scale 0.5\n");


    fprintf(stdout,"%s\n", GetDXTCVersion());
    fprintf(stdout,"Send comments, bug fixes and feature requests to doug@nvidia.com\n");

}





HRESULT callback(void * data, int miplevel, DWORD size)
{
	DWORD * ptr = (DWORD *)data;
	for(int i=0; i< size/4; i++)
	{
		DWORD c = *ptr++;
	}

	
	return 0;
}





HRESULT nvDXT::process_command_line(int argc, char * argv[])
{
    
    int i;
    i = 1;
    while(i<argc)
    {
        char * token = argv[i];
        

        if (strcmp(token, "-binaryalpha") == 0)
        {
            bBinaryAlpha = true;
        }
        else if (strcmp(token, "-alpha_threshold") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            BinaryAlphaThreshold = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-alphaborder") == 0)
        {
            bAlphaBorder = true;
        }
        else if (strcmp(token, "-flip") == 0)
        {
            bFlipTopToBottom = true;
        }
        else if (strcmp(token, "-deep") == 0)
        {
            
            bDeep = true;


            char * type = 0;
            
            if (i + 1 < argc)
                type = argv[i+1];

            if (type && type[0] != '-')
            {
                recursive_dirname = argv[i+1];
                i++;
            }
            else
            {
                char cwd[MAX_PATH_NAME];
                char * t;
                t = _getcwd(cwd, MAX_PATH_NAME);
                recursive_dirname = t;
        
            }



        }
        else if (strcmp(token, "-rescale") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            char * type = argv[i+1];
            
            if (strcmp(type, "nearest") == 0)
                rescaleImageType = RESCALE_NEAREST_POWER2;
            else if (strcmp(type, "hi") == 0)
                rescaleImageType = RESCALE_BIGGEST_POWER2;
            else if (strcmp(type, "lo") == 0)
                rescaleImageType = RESCALE_SMALLEST_POWER2;
            else if (strcmp(type, "next_lo") == 0)
                rescaleImageType = RESCALE_NEXT_SMALLEST_POWER2;
            else
            {
                fprintf(stdout, "Unknown option rescale'%s'\n", type);
                return ERROR_BAD_OPTION;
            }
            i++;
        }
        else if (strcmp(token, "-rel_scale") == 0)
        {

            if (i + 2 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            
            rescaleImageType = RESCALE_RELSCALE;

            scaleX = atof(argv[i+1]);
            scaleY = atof(argv[i+2]);
            i +=2;

        }
        else if (strcmp(token, "-clamp") == 0)
        {

            if (i + 2 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            rescaleImageType = RESCALE_CLAMP;

            scaleX = atof(argv[i+1]);
            scaleY = atof(argv[i+2]);
            i +=2;

        }
        else if (strcmp(token, "-border") == 0)
        {
            bBorder = true;
        }
        else if (strcmp(token, "-fadeamount") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            FadeAmount = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-nmips") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            SpecifiedMipMaps = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-force4") == 0)
        {
            bForceDXT1FourColors = true;
        }
        else if (strcmp(token, "-bordercolor") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            BorderColor.u = strtoul(argv[i+1], 0, 16);
            i++;
        }
        else if (strcmp(token, "-rgbe") == 0)
        {
            bRGBE = true;
        }
        else if (strcmp(token, "-fadealpha") == 0)
        {
            bFadeAlpha = true;
        }

        else if (strcmp(token, "-fadetoalpha") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            FadeToAlpha = atoi(argv[i+1]);
            i++;
        }

        else if (strcmp(token, "-fadecolor") == 0)
        {
            bFadeColor = true;
        }
        else if (strcmp(token, "-fadetocolor") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            FadeToColor.u = strtoul(argv[i+1], 0, 16);
            i++;
        }
        else if (strcmp(token, "-nomipmap") == 0)
        {
            MipMapType = dNoMipMaps;
            //bGenMipMaps = false;
        }
        else if (strcmp(token, "-dxt1a") == 0)
        {
            TextureFormat =  kDXT1a;
        }
        else if ((strcmp(token, "-dxt1c") == 0) || (strcmp(token, "-dxt1") == 0))
        {
            TextureFormat =  kDXT1;
        }
        else if (strcmp(token, "-dxt3") == 0)
        {
            TextureFormat =  kDXT3;
        }
        else if (strcmp(token, "-dxt5") == 0)
        {
            TextureFormat =  kDXT5;
        }
        else if (strcmp(token, "-cubemap") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            bCubeMap = true;
            cubeMapName = argv[i+1];
            i++;
        }
        else if (strcmp(token, "-volume") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            bVolumeTexture = true;
            volumeTextureName = argv[i+1];
            i++;
        }
        else if (strcmp(token, "-swap") == 0)
        {
            bSwapRGB = true;
        }
        else if (strcmp(token, "-signed") == 0)
        {
            bSigned = true;
        }
        else if (strcmp(token, "-quick") == 0)
        {
            bQuickCompress = true;
        }
        else if (strcmp(token, "-profile") == 0)
        {
            HRESULT hr = SetProfileName(argv[i+1]);
            if (hr != 0)
            {
                printf("Can't open profile %s\n",argv[i+1]);
                exit(ERROR_CANT_OPEN_PROFILE);
            }

            i++;

        }
        else if (strcmp(token, "-all") == 0)
        {
            all_files = true;
        }

        
        else if (strcmp(token, "-dither") == 0)
        {
            bDitherColor = true;
        }

        else if (strcmp(token, "-sharpenMethod") == 0)
        {
             
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            char * sharpenFilterName = argv[i+1];

            if (strcmp(sharpenFilterName, "None") == 0)
                SharpenFilterType = kSharpenFilterNone;
            else if (strcmp(sharpenFilterName, "Negative") == 0)
                SharpenFilterType = kSharpenFilterNegative;
           else if (strcmp(sharpenFilterName, "Lighter") == 0)
                SharpenFilterType = kSharpenFilterLighter;
           else if (strcmp(sharpenFilterName, "Darker") == 0)
                SharpenFilterType = kSharpenFilterDarker;
           else if (strcmp(sharpenFilterName, "ContrastMore") == 0)
                SharpenFilterType = kSharpenFilterContrastMore;
           else if (strcmp(sharpenFilterName, "ContrastLess") == 0)
                SharpenFilterType = kSharpenFilterContrastLess;
           else if (strcmp(sharpenFilterName, "Smoothen") == 0)
                SharpenFilterType = kSharpenFilterSmoothen;
           else if (strcmp(sharpenFilterName, "SharpenSoft") == 0)
                SharpenFilterType = kSharpenFilterSharpenSoft;
           else if (strcmp(sharpenFilterName, "SharpenMedium") == 0)
                SharpenFilterType = kSharpenFilterSharpenMedium;
           else if (strcmp(sharpenFilterName, "SharpenStrong") == 0)
                SharpenFilterType = kSharpenFilterSharpenStrong;
           else if (strcmp(sharpenFilterName, "FindEdges") == 0)
                SharpenFilterType = kSharpenFilterFindEdges;
           else if (strcmp(sharpenFilterName, "Contour") == 0)
                SharpenFilterType = kSharpenFilterContour;
           else if (strcmp(sharpenFilterName, "EdgeDetect") == 0)
                SharpenFilterType = kSharpenFilterEdgeDetect;
           else if (strcmp(sharpenFilterName, "EdgeDetectSoft") == 0)
                SharpenFilterType = kSharpenFilterEdgeDetectSoft;
           else if (strcmp(sharpenFilterName, "Emboss") == 0)
                SharpenFilterType = kSharpenFilterEmboss;
           else if (strcmp(sharpenFilterName, "MeanRemoval") == 0)
                SharpenFilterType = kSharpenFilterMeanRemoval;
           else if (strcmp(sharpenFilterName, "UnSharp") == 0)
           {
                SharpenFilterType = kSharpenFilterUnSharp;
                //
                if (i + 3 >= argc)
                {
                    printf("incorrect number of arguments for kSharpenFilterUnSharp\n");
                    exit(1);
                }
                double radius = atof(argv[i+1]);
                double amount = atof(argv[i+2]);
                int threshold = atoi(argv[i+3]);
                

                unsharp_init(radius, amount, threshold);
                i += 3;


           }




           else if (strcmp(sharpenFilterName, "XSharpen") == 0)
           {
                SharpenFilterType = kSharpenFilterXSharpen;
                if (i + 2 >= argc)
                {
                    printf("incorrect number of arguments for kSharpenFilterXSharpen\n");
                    exit(1);
                }
                int xsharpen_strength = atoi(argv[i+1]);
                int xsharpen_threshold = atoi(argv[i+2]);

               xsharpen_init(xsharpen_strength, xsharpen_threshold);

           }
           //else if (strcmp(sharpenFilterName, "FilterWarpSharp") == 0)
           //     SharpenFilterType = kFilterWarpSharp;
           else if (strcmp(sharpenFilterName, "Custom") == 0)
           {
                SharpenFilterType = kSharpenFilterCustom;
           }
           else
           {
               printf("Unrecognized sharpen filter %s\n", sharpenFilterName);
               exit(1);
           }

            i++;
        }

        /*else if (strcmp(token, "-lambda") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenLambda = atof(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-sscale") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenScale = atof(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-mu") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenMu = atof(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-sb") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenBlur = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-sf2") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenFlavor2 = atof(argv[i+1]);
            i++;
        }       
        else if (strcmp(token, "-theta") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            sharpenTheta = atof(argv[i+1]);
            i++;
        }        
        else if (strcmp(token, "-edge_radius") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            edgeRadius = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-tc") == 0)
        {
            bTwoComponents = true;
        }
        else if (strcmp(token, "-nms") == 0)
        {
            bNonMaximalSuppression = true;
        } */




        else if (strcmp(token, "-outdir") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            output_dirname = argv[i+1];
            i++;

            bUserSpecifiedOutputDir = true;
        }
        else if (strcmp(token, "-outsamedir") == 0)
        {
            bOutputSameDir = true;
        }
        else if (strcmp(token, "-overwrite") == 0)
        {
            bOverwrite = true;
        }
        else if (strcmp(token, "-u4444") == 0)
        {
            TextureFormat =  k4444;
        }
        else if (strcmp(token, "-u1555") == 0)
        {
            TextureFormat =  k1555;
        }
        else if (strcmp(token, "-u565") == 0)
        {
            TextureFormat =  k565;
        }
        else if (strcmp(token, "-u8888") == 0)
        {
            TextureFormat =  k8888;
        }
        else if (strcmp(token, "-fp32x4") == 0)
        {
            TextureFormat =  kA32B32G32R32F;
        }
        else if (strcmp(token, "-fp32") == 0)
        {
            TextureFormat =  kR32F;
        }
        else if (strcmp(token, "-fp16x4") == 0)
        {
            TextureFormat =  kA16B16G16R16F;
        }
        else if (strcmp(token, "-u888") == 0)
        {
            TextureFormat =  k888;
        }
        else if (strcmp(token, "-u555") == 0)
        {
            TextureFormat =  k555;
        }
        else if (strcmp(token, "-p8") == 0)
        {
            TextureFormat =  k8;
        }
        else if (strcmp(token, "-p4") == 0)
        {
            TextureFormat =  k4;
        }
        else if (strcmp(token, "-a8") == 0)
        {
            TextureFormat =  kA8;
        }
        else if (strcmp(token, "-A8L8") == 0)
        {
            TextureFormat =  kA8L8;
        }
        else if (strcmp(token, "-L8") == 0)
        {
            TextureFormat =  kL8;
        }
        else if (strcmp(token, "-timestamp") == 0)
        {
            timestamp = true;
        }
        else if (strcmp(token, "-list") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            list = true;
            
            listfile = argv[i+1];
            i++;
        }




        else if (strcmp(token, "-Point") == 0)
        {
            MIPFilterType = kMIPFilterPoint;
        }
        else if (strcmp(token, "-Box") == 0)
        {
            MIPFilterType = kMIPFilterBox;
        }
        else if (strcmp(token, "-Triangle") == 0)
        {
            MIPFilterType = kMIPFilterTriangle;
        }
        else if (strcmp(token, "-Quadratic") == 0)
        {
            MIPFilterType = kMIPFilterQuadratic;
            
        }
       else if (strcmp(token, "-Cubic") == 0)
        {
            MIPFilterType = kMIPFilterCubic;
            
        }
       else if (strcmp(token, "-Catrom") == 0)
        {
            MIPFilterType = kMIPFilterCatrom;
            
        }
       else if (strcmp(token, "-Mitchell") == 0)
        {
            MIPFilterType = kMIPFilterMitchell;
            
        }
       else if (strcmp(token, "-Gaussian") == 0)
        {
            MIPFilterType = kMIPFilterGaussian;
            
        }
       else if (strcmp(token, "-Sinc") == 0)
        {
            MIPFilterType = kMIPFilterSinc;
            
        }
       else if (strcmp(token, "-Bessel") == 0)
        {
            MIPFilterType = kMIPFilterBessel;
            
        }
      else if (strcmp(token, "-Hanning") == 0)
        {
            MIPFilterType = kMIPFilterHanning;
            
        }
      else if (strcmp(token, "-Hamming") == 0)
        {
            MIPFilterType = kMIPFilterHamming;
            
        }
      else if (strcmp(token, "-Blackman") == 0)
        {
            MIPFilterType = kMIPFilterBlackman;
            
        }
      else if (strcmp(token, "-Kaiser") == 0)
        {
            MIPFilterType = kMIPFilterKaiser;
            
        }
      else if (strcmp(token, "-n4") == 0)
      {
          kerneltype = dFilter4x;
          bNormalMap = true;
          bNormalMapConversion = true;
        }
        else if (strcmp(token, "-n3x3") == 0)
        {
            kerneltype = dFilter3x3;
            bNormalMapConversion = true;
            bNormalMap = true;
        }
        else if (strcmp(token, "-n5x5") == 0)
        {
            kerneltype = dFilter5x5;                               
            bNormalMapConversion = true;
            bNormalMap = true;
        }
        else if (strcmp(token, "-n7x7") == 0)
        {
            kerneltype = dFilter7x7;                               
            bNormalMapConversion = true;
            bNormalMap = true;
        }
        else if (strcmp(token, "-n9x9") == 0)
        {
            kerneltype = dFilter9x9;                               
            bNormalMapConversion = true;
            bNormalMap = true;
        }
        else if (strcmp(token, "-dudv") == 0)
        {
            kerneltype = dFilterDuDv;
            bNormalMapConversion = true;
            bNormalMap = false;
            TextureFormat =  kV8U8;
        }
        else if (strcmp(token, "-cxv8u8") == 0)
        {
            kerneltype = dFilterDuDv;
            bNormalMapConversion = true;
            bNormalMap = false;
            TextureFormat =  kCxV8U8;
        }
        else if (strcmp(token, "-depth") == 0)
        {
            //depthtype = 1;
            bDepthConversion = true;
        }
        else if (strcmp(token, "-to_height") == 0)
        {
            bToHeightMapConversion = true;
            toHeightScale = atof(argv[i+1]);
            toHeightBias = atof(argv[i+2]);
            i +=2;

        }
        else if (strcmp(token, "-alpha") == 0)
        {
            colorcnv = dALPHA;
        }
        else if (strcmp(token, "-rgb") == 0)
        {
            colorcnv = dAVERAGE_RGB;
        }
        else if (strcmp(token, "-biased") == 0)
        {

            colorcnv = dBIASED_RGB;
        }
        else if (strcmp(token, "-red") == 0)
        {
            colorcnv = dRED;
        }
        else if (strcmp(token, "-green") == 0)
        {
            colorcnv = dGREEN;
        }
        else if (strcmp(token, "-blue") == 0)
        {
            colorcnv = dBLUE;
        }
        else if (strcmp(token, "-max") == 0)
        {
            colorcnv = dMAX;
        }
        else if (strcmp(token, "-colorspace") == 0)
        {
            colorcnv = dCOLORSPACE;
        }
        else if (strcmp(token, "-norm") == 0)
        {
            colorcnv = dNORMALIZE;
            bNormalMap = true;

        }
        else if (strcmp(token, "-diffusion") == 0)
        {
            bErrorDiffusion = true;

        }
        else if (strcmp(token, "-alphamod") == 0)
        {
            bAlphaModulate = true;

        }        
        else if (strcmp(token, "-aheight") == 0)
        {
            alphaResult = dAlphaHeight;
        }
        else if (strcmp(token, "-aclear") == 0)
        {
            alphaResult = dAlphaClear;
        }
        else if (strcmp(token, "-awhite") == 0)
        {
            alphaResult = dAlphaWhite;
        }
            
        else if (strcmp(token, "-file") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            specified_file = argv[i+1];

            if ( (specified_file.find("\\")  != -1) ||   (specified_file.find("/")  != -1) )
                bFullPathSpecified = true;

            i++;
        }
        else if (strcmp(token, "-output") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            output_specified_file = argv[i+1];

            bOutputFileSpecified = true;

            i++;
        }
        else if (strcmp(token, "-append") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            output_append = argv[i+1];

            bOutputFileAppend = true;

            i++;
        }

        else if (strcmp(token, "-24") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            char * mode = argv[i+1];
            i++;
            
            if (strcmp(mode, "dxt1c") == 0)
            {
                Mode24 = kDXT1;
            }
            else if (strcmp(mode, "dxt1") == 0)
            {
                Mode24 = kDXT1;
            }
            else if (strcmp(mode, "dxt1a") == 0)
            {
                Mode24 = kDXT1a;
            }
            else if (strcmp(mode, "dxt3") == 0)
            {
                Mode24 = kDXT3;
            }
            
            else if (strcmp(mode, "dxt5") == 0)
            {
                Mode24 = kDXT5;
            }
            
            else if (strcmp(mode, "u1555") == 0)
            {
                Mode24 = k1555;
            }
            
            else if (strcmp(mode, "u4444") == 0)
            {
                Mode24 = k4444;
            }
            
            else if (strcmp(mode, "u565") == 0)
            {
                Mode24 = k565;
            }
            
            else if (strcmp(mode, "u8888") == 0)
            {
                Mode24 = k8888;
            }
            else if (strcmp(mode, "u888") == 0)
            {
                Mode24 = k888;
            }
            else if (strcmp(mode, "u555") == 0)
            {
                Mode24 = k555;
            }
            else
            {
                printf("Unknown format %s, using DXT1\n", mode);
                Mode24 = kDXT1;
            }

        }
        else if (strcmp(token, "-32") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            char * mode = argv[i+1];
            i++;
            
            if (strcmp(mode, "dxt1c") == 0)
            {
                Mode32 = kDXT1;
            }
            if (strcmp(mode, "dxt1") == 0)
            {
                Mode32 = kDXT1;
            }
            else if (strcmp(mode, "dxt1a") == 0)
            {
                Mode32 = kDXT1a;
            }
            else if (strcmp(mode, "dxt3") == 0)
            {
                Mode32 = kDXT3;
            }
            
            else if (strcmp(mode, "dxt5") == 0)
            {
                Mode32 = kDXT5;
            }
            
            else if (strcmp(mode, "u1555") == 0)
            {
                Mode32 = k1555;
            }
            
            else if (strcmp(mode, "u4444") == 0)
            {
                Mode32 = k4444;
            }
            
            else if (strcmp(mode, "u565") == 0)
            {
                Mode32 = k565;
            }
            
            else if (strcmp(mode, "u8888") == 0)
            {
                Mode32 = k8888;
            }
            else if (strcmp(mode, "u888") == 0)
            {
                Mode32 = k888;
            }
            else if (strcmp(mode, "u555") == 0)
            {
                Mode32 = k555;
            }
            else
            {
                printf("Unknown format %s, using DXT1\n", mode);
                Mode32 = kDXT1;
            }

        }
        else if (strcmp(token, "-scale") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }

            scale = atof(argv[i+1]);
            i++;
                
        }
        else if (strcmp(token, "-prescale") == 0)
        {
            if (i + 2 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            rescaleImageType = RESCALE_PRESCALE;
            scaleX = atof(argv[i+1]);
            scaleY = atof(argv[i+2]);
            i+=2;
                
        }

        else if (strcmp(token, "-wrap") == 0)
        {
            wrap = true;
                
        }
        else if (strcmp(token, "-minz") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("incorrect number of arguments\n");
                exit(1);
            }
            minz = atoi(argv[i+1]);
            i++;
                
        }
        else if (token[0] == '-')
        {
            fprintf(stdout, "Unknown option '%s'\n", token);
            return ERROR_BAD_ARG;
            
        }
        else
        {
            //  must be a file name
            specified_file = argv[i];
            if ( (specified_file.find("\\")  != -1) ||   (specified_file.find("/")  != -1) )
                bFullPathSpecified = true;
        }
        i++;
    }
    return 0;
} 
   


// input_directory 
HRESULT  nvDXT::ProcessDirectory()
{

    HRESULT hr; 
    if (all_files)
    {
        hr = compress_all();
    }
    else if (specified_file.find("*") != -1)
    {
        hr = compress_some(const_cast<char *>(specified_file.c_str()));
    }

    else if (specified_file.size() > 0)
    {
        //inputfile = argv[optind];]


        hr = compress_image_file(const_cast<char *>(specified_file.c_str()), TextureFormat);
    }
    else
        fprintf(stdout, "nothing to do\n");

    return hr;

}
 

void WriteDTXnFile(DWORD count, void *buffer, void * userData)
{
    _write(fileout, buffer, count);

}


void ReadDTXnFile(DWORD count, void *buffer, void * userData)
{
        
    _read(filein, buffer, count);

}


typedef Vec4f float4;

void rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4]);
void float2rgbe(unsigned char rgbe[4], float R, float G, float B);
void float2rgbe(float4 & rgbe, float R, float G, float B);




int main(int argc, char * argv[])
{
    /*
   float red, green, blue; 
   float red2, green2, blue2; 
   unsigned char rgbe[4];
   unsigned char rgbe2[4];


   rgbe[0] = 10;
   rgbe[1] = 20;
   rgbe[2] = 30;
   rgbe[3] = 128;

   rgbe2float(&red, &green, &blue, rgbe);   


   //float2rgbe(rgbe2, red, green, blue);
   
   float2rgbe(rgbe2, red, green, blue);


   rgbe2float(&red2, &green2, &blue2, rgbe2);   


   exit(0);*/

    



    nvDXT dxt;
    /*int cw = _controlfp( 0, 0 );

    // Set the exception masks off, turn exceptions on

    //cw &=~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT | EM_ZERODIVIDE | EM_DENORMAL);
    //cw &=~(EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE | EM_DENORMAL);
    cw &=~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL);

    // Set the control word

    _controlfp( cw, MCW_EM );*/

#ifdef NVDXTDLL
    SetReadDTXnFile(ReadDTXnFile);
    SetWriteDTXnFile(WriteDTXnFile);
#endif


    dxt.specified_file = "";

    if (argc == 1)
    {
        usage();
        return ERROR_BAD_ARG;
    }

    dxt.output_dirname = ".";

    char cwd[MAX_PATH_NAME];
    char * startdir;
    startdir = _getcwd(cwd, MAX_PATH_NAME);
    dxt.input_dirname = startdir;

    dxt.MIPFilterType = kMIPFilterMitchell;    


    HRESULT hr = dxt.process_command_line(argc, argv);
    if (hr < 0)
    {
        return ERROR_BAD_ARG;
    }

     
    if (dxt.bUserSpecifiedOutputDir == true)
    {
        
        char cwd[MAX_PATH_NAME];
        char * t;
        t = _getcwd(cwd, MAX_PATH_NAME);


        int md = _chdir(dxt.output_dirname.c_str());
        
        _chdir(cwd);

        if (md != 0)
        {
            md = _mkdir(dxt.output_dirname.c_str());
            
            if (md == 0)
            {
                fprintf(stdout, "directory %s created\n", dxt.output_dirname.c_str());
                fflush(stdout);
            }
            else if (errno != EEXIST)
            {
                fprintf(stdout, "problem with output directory %s\n", dxt.output_dirname.c_str());
                return ERROR_BAD_OUTPUT_DIRECTORY;
            } 
            else
            {
                fprintf(stdout, "output directory %s\n", dxt.output_dirname.c_str());
                fflush(stdout);
            }
        }
    }


    hr = 0;



    if (dxt.list)
    {
        if (dxt.bVolumeTexture)
        {
            dxt.compress_volume_from_list();
        }
        else if (dxt.bCubeMap)
            hr = dxt.compress_cubemap_from_list();
        else
            hr = dxt.compress_list();
    }
    else if (dxt.bDeep)
    {
        
        dxt.compress_recursive(dxt.recursive_dirname.c_str(), hr);
        //compress_recursive("c:\\");
    }
    else
        hr = dxt.ProcessDirectory();

    
    return hr;
}











#include "sharpen.h"    

void unsharp_region
  (GPixelRgn &srcPR,
   GPixelRgn &destPR,
   int width,
   int height,
   int bytes,
   double radius,
   double amount,
   int threshold,
   int x1,int x2,int y1,int y2);



                 


/*void ReadDTXnFile (DWORD count, void *buffer)
{
    // stubbed, we are not reading files
}*/





HRESULT nvDXT::compress_cubemap_from_list()
{
    
    FILE *fp = fopen( listfile.c_str(), "r");
    
    if (fp == 0)
    {
        fprintf(stdout, "Can't open list file '%s'\n", listfile.c_str());
        return ERROR_CANT_OPEN_LIST_FILE;
    }
    
    char buff[MAX_PATH_NAME];

    int i = 0;
    while(fgets(buff, MAX_PATH_NAME, fp))
    {      
        // has a crlf at the end
        int t = strlen(buff);
        buff[t - 1] = 0;

        cube_names[i] = buff;
        i++;

    }

    fclose(fp);

    if (i != 6)
    {
        fprintf(stdout, "There are not six images in listfile '%s'\n", listfile.c_str());
        return ERROR_BAD_LIST_FILE_CONTENTS;
    }

    compress_cube();

    return 0;


}


inline void AlphaAndVectorToARGB( const unsigned long & theHeight, const Vec3 & inVector, unsigned long& outColor )
{
	int red   = (( inVector.x + 1.0f ) * 127.5f ) + 0.5;
	int green = (( inVector.y + 1.0f ) * 127.5f ) + 0.5 ;
	int blue  = (( inVector.z + 1.0f ) * 127.5f ) + 0.5 ;

    if ( red < 0)
        red = 0;
    if (red > 255)
        red = 255;
    if (green < 0)
        green = 0;
    if (green > 255)
        green = 255;

    if (blue < 0)
        blue = 0;
    if (blue > 255)
        blue = 255;

	outColor = ( ( (unsigned int)theHeight << 24 ) | ( red << 16 ) | ( green << 8 ) | ( blue << 0 ) );
}


inline void ARGBToAlphaAndVector(const unsigned long& inColor, unsigned long& theHeight, Vec3& outVector)
{

    DWORD r, g, b, a;

    a = (inColor >> 24) & 0xFF;
    r = (inColor >> 16) & 0xFF;
    g = (inColor >>  8) & 0xFF;
    b = (inColor >>  0) & 0xFF;
                        
    outVector.x = (float)r / 127.5 - 1.0;
    outVector.y = (float)g / 127.5 - 1.0;
    outVector.z = (float)b / 127.5 - 1.0;

    theHeight = a;

}


/*
void ConvertNormalMapToHeight(float scale, float bias, int w, int h, void * vdata) 
{

	DWORD *data = (DWORD *)vdata;
	float *heightX = new float[w * h];
	float *heightY = new float[w * h];

    float *hptrX = heightX; 
    float *hptrY = heightY; 
    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            *hptrX++ = 0;
            *hptrY++ = 0;
        }
    }



    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            Vec3 n;
            DWORD a;
            
            ARGBToAlphaAndVector(data[y * w + x], a, n);
            //n.normalize();


            float pre;
            if ( x == 0)
                pre = 0;
            else
                pre = heightX[y * w + x - 1];

            float * dst = &heightX[y * w + x];

            if (n.z == 0)
            {
                int t = 1;
            }
            else 
            {
                float slope = n.x / n.z;

                *dst = pre + slope;
            }
        }
    }

    for ( int x = 0; x < w; x++ )
    {

        for ( int y = 0; y < h; y++ )
        {
            Vec3 n;
            DWORD a;

            ARGBToAlphaAndVector(data[y * w + x], a, n);
            //n.normalize();


            float pre;
            if ( y == 0)
                pre = 0;
            else
                pre = heightY[(y-1) * w + x];

            float * dst = &heightY[y * w + x];

            if (n.z == 0)
            {
                int t = 1; // not chage
            }
            else 
            {
                float slope = n.y / n.z;

                *dst = pre + slope;
            }
        }
    }



    hptrX = heightX; 
    hptrY = heightY; 
    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {

            *hptrX = (*hptrX + *hptrY) / 2.0;

            *hptrX++;
            *hptrY++;
        }

    }




    float *hptr = heightX; 
    float _min = *hptr;
    float _max = *hptr;

    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            float h = *hptr++;

            if (h < _min)
                _min = h; 
            if (h > _max)
                _max = h; 
        }
    
    }
    float Zscale;

    if (_max == _min)
        Zscale = 1;
    else
        Zscale = 1./(_max - _min);



	unsigned char *ptr = (unsigned char *)vdata;
    
    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            float h = heightX[y * w + x];

            // convert to [0-1]
            h = (h - _min) * Zscale;

            h = h * scale + bias;

            // convert to color
            h *= 255.0 + 0.5;
            
            int grey = h;
            if (grey > 255)
                grey = 255;
            if (grey < 0)
                grey = 0;

            *ptr++ = h;
            *ptr++ = h;
            *ptr++ = h;
            *ptr++ = 0xFF;

        }
    }




    delete [] heightX;
    delete [] heightY;

}*/
//////////////////////////////////////////////////////

