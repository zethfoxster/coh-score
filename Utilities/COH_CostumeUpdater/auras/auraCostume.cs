using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.auras
{
    public class auraCostume
    {
        string includeHeader = "\tInclude";
        string root;   // file root
        public string fullPath;
        public string includeStatement;
        public string fileName;                        // Standard.ctm file
        public string charType;                        // male, female, huge ...
        public string config;                          // All, All - old, Common, Mutation, Natural, Technology
        public string bodyPart;                        // Head, Upper Body, Lower Body
        public bool exists;

        public auraCostume(string includeLine, string partOfBody, string typeOfChar, string pathRoot)
        {
            root = pathRoot;
            int startIndex = includeHeader.Length;
            int nameLen = includeLine.Length - startIndex;
            string includeRelativePath = includeLine.Substring(startIndex + 1, nameLen - 1);
            includeStatement = includeLine;
            fileName = System.IO.Path.GetFileName(includeRelativePath);
            fullPath = root + includeRelativePath;
            charType = typeOfChar;
            bodyPart = partOfBody;
            exists = System.IO.File.Exists(fullPath);
            config = "";

        }
    }
}
