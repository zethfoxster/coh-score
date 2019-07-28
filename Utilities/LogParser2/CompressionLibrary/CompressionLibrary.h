// CompressionLibrary.h

#pragma once

using namespace System;

namespace CompressionLibrary {

	public ref class GZipper
	{
	public:
		static void Read(String^ gzipFileName, String^ outputFileName);
		static void Write(String^ inputFileName, String^ gzipFileName);
	};
}
