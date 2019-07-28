using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater
{
    public class Region
    {
        private string root;
        string includeHeader = "\tInclude";
        public string name;
        public string displayName;
        public string key;
        public string ctType;
        public string bodyPart;

        public bool isHead;
        public bool isUpperBody;
        public bool islowerBody;
        public bool isCostume;
        public bool isWeapon;
        public bool isShield;
        public bool isCape;
        public bool isSpecial;
        public bool isAura;
        public bool hasKey;

        public ArrayList includeCostumes;

        public Region(string fileType, string pathRoot)
        {
            root = pathRoot;
            initialize();
            ctType = fileType;
        }
        private void initialize()
        {
            isHead = false;
            isUpperBody = false;
            islowerBody = false;
            isCostume = false;
            isWeapon = false;
            isShield = false;
            isCape = false;
            isSpecial = false;
            isAura = false;
            hasKey = false;
            includeCostumes = new ArrayList();
            name = "";
            displayName = "";
            ctType = "";
            bodyPart = "";
        }
        public void addInclude(string includeLine)
        {
            setBoolTypes();
            bodyPart = name.Replace(" ", "_");
            COH_CostumeUpdater.auras.auraCostume ctm = new COH_CostumeUpdater.auras.auraCostume(includeLine, bodyPart, ctType, root);
            includeCostumes.Add(ctm);
        }
        private void setBoolTypes()
        {
            switch (bodyPart)
            {
                case "Head":
                    isHead = true;
                    break;
                case "Upper_Body":
                    isUpperBody = true;
                    break;
                case "Lower_Body":
                    islowerBody = true;
                    break;
                case "Weapons":
                    isWeapon = true;
                    break;
                case "Shields":
                    isShield = true;
                    break;
                case "Capes":
                    isCape = true;                
                    break;
                case "Special":
                    isSpecial = true;
                    break;
            }
            if (displayName.Equals("Auras"))
                isAura = true;
        }

    }

}
