
#include "arda2/storage/stoFirst.h"
#include "arda2/core/corStlAlgorithm.h"
#include "arda2/storage/stoProperty.h"
#include "arda2/storage/stoConfigFile.h"


#include "arda2/properties/proClassNative.h"

using namespace std;
PRO_REGISTER_ABSTRACT_CLASS(stoPropertyFilename, proPropertyString)
PRO_REGISTER_ABSTRACT_CLASS(stoPropertyDirectory, proPropertyString)
PRO_REGISTER_ABSTRACT_CLASS(stoPropertyFullpath, proPropertyString)

PRO_REGISTER_ABSTRACT_CLASS(stoPropertyFilenameNative, stoPropertyFilename)
PRO_REGISTER_ABSTRACT_CLASS(stoPropertyDirectoryNative, stoPropertyDirectory)
PRO_REGISTER_ABSTRACT_CLASS(stoPropertyFullpathNative, stoPropertyFullpath)

PRO_REGISTER_CLASS(stoPropertyFilenameOwner, stoPropertyFilename)
PRO_REGISTER_CLASS(stoPropertyDirectoryOwner, stoPropertyDirectory)
PRO_REGISTER_CLASS(stoPropertyFullpathOwner, stoPropertyFullpath)


/**
 * this ensures that only the filename after the final slash is kept
**/
void stoPropertyFilename::FromString( proObject* o, const string& aValue )
{
    char *value = const_cast<char *>(aValue.c_str());
    char *p = NULL;
    
    // check for forward slash 
    p = strrchr(value, '/');
    if (!p)
    {
        // check for back slash
        p = strrchr(value, '\\');
    }
    if (p)
    {
        p++; // skip slash character
    }
    else
    {
        p = value; // or, use the entire string.
    }
  
    SetValue(o, p);
}

void proRegisterStorageTypes()
{
    IMPORT_PROPERTY_CLASS(stoPropertyFilename);
    IMPORT_PROPERTY_CLASS(stoPropertyFullpath);
    IMPORT_PROPERTY_CLASS(stoPropertyDirectory);

    IMPORT_PROPERTY_CLASS(stoPropertyFilenameOwner);
    IMPORT_PROPERTY_CLASS(stoPropertyFullpathOwner);
    IMPORT_PROPERTY_CLASS(stoPropertyDirectoryOwner);
}



static const char *proAnnoStr_title = "PropertyAnnotations";

/**
 * wrapper for a filename instead of config file
 */
errResult stoLoadAnnotations(const string &filename)
{
    stoConfigFile file;
    if (ISOK(file.Load(filename)))
    {
        return stoLoadAnnotations(file);
    }
    return ER_Failure;
}

// helper function for loading annotations
void VerifyUniqueClasses(vector<string> &classNames)
{
    for (size_t i=0; i != classNames.size(); ++i)
    {
        string &className = classNames[i];
        for (size_t j = i+1; j < classNames.size(); ++j)
        {
            string &matchName = classNames[j];
            if (className == matchName)
            {
                vector<string>::iterator it = classNames.begin();
                advance(it, j);
                classNames.erase(it);
                --j;
            }
        }
    }
}

/**
 * reads property annotations from an .ini file and 
 * applies them to the class registry
 */
errResult stoLoadAnnotations(stoConfigFile &config)
{
    // iterate through classes 
    vector<string> classNames;
    config.GetMultiValues(proAnnoStr_title, "Class", classNames);
    VerifyUniqueClasses(classNames);
    for (vector<string>::iterator cit = classNames.begin(); cit != classNames.end(); cit++)
    {
        string classSectionName("Class_");
        classSectionName += (*cit);
        proClass *annoClass = proClassRegistry::ClassForName<proClass*>(*cit);
        if (annoClass)
        {
            // iterate through properties of each class
            vector<string> propNames;
            config.GetMultiValues(classSectionName, "Property", propNames);
            for (vector<string>::iterator pit = propNames.begin(); pit != propNames.end(); pit++)
            {
                proProperty *annoProperty = annoClass->GetPropertyByName(*pit);
                if (annoProperty)
                {
                    string propertySectionName(classSectionName);
                    propertySectionName += "_";
                    propertySectionName += (*pit);

                    // get the annotations for the property, and build anno string
                    stoConfigFileSection *section = config.GetSection(propertySectionName);
                    if (section)
                    {
                        string annoString;
                        stoConfigFileSection::LineVector lines = section->GetLines();
                        for (stoConfigFileSection::LineVector::iterator lit = lines.begin(); lit != lines.end(); lit++)
                        {
                            stoConfigFileSection::Line &line = (*lit);
                            annoString += line.key;
                            annoString += "|";
                            annoString += line.value;
                            annoString += "|";
                        }
                        annoProperty->SetAnnotations(annoString.c_str());
                    }
                }
                else
                {
                    ERR_REPORTV(ES_Warning, ("Annotation property %s of class %s not found.", (*pit).c_str(), (*cit).c_str()));
                }
            }
        }
        else
        {
            ERR_REPORTV(ES_Warning, ("Annotated class %s not found", (*cit).c_str()));
        }
    }
    return ER_Success;
}

/**
 * takes the property annotation data from the source file and adds
 * it to the destination file.
 */
errResult stoMergeAnnotations(stoConfigFile &source, stoConfigFile &destination)
{
    stoConfigFileSection *topSection = destination.GetSection(proAnnoStr_title);
    if (!topSection)
    {
        return ER_Failure;
    }

    // iterate through classes 
    vector<string> classNames;
    source.GetMultiValues(proAnnoStr_title, "Class", classNames);
    VerifyUniqueClasses(classNames);
    for (vector<string>::iterator cit = classNames.begin(); cit != classNames.end(); cit++)
    {
        string classSectionName("Class_");
        classSectionName += (*cit);

        // add this class to destination 
        topSection->SetValue("Class", (*cit), true, false);
        stoConfigFileSection *classSection = destination.GetSection(classSectionName);
        if (!classSection)
        {
            return ER_Failure;
        }

        // iterate through properties of each class
        vector<string> propNames;
        source.GetMultiValues(classSectionName, "Property", propNames);

        for (vector<string>::iterator pit = propNames.begin(); pit != propNames.end(); pit++)
        {
            string propertySectionName(classSectionName);
            propertySectionName += "_";
            propertySectionName += (*pit);

            classSection->SetValue("Property", (*pit), true, false);

            stoConfigFileSection *propertySection = destination.GetSection(propertySectionName);
            if (!propertySection)
            {
                return ER_Failure;
            }

            // get the annotations for the property, and add to destination
            stoConfigFileSection *section = source.GetSection(propertySectionName);
            if (section)
            {
                string annoString;
                stoConfigFileSection::LineVector lines = section->GetLines();
                for (stoConfigFileSection::LineVector::iterator lit = lines.begin(); lit != lines.end(); lit++)
                {
                    stoConfigFileSection::Line &line = (*lit);
                    propertySection->SetValue(line.key, line.value);
                }
            }
        }
    }
    return ER_Success;
}

/**
 * save the annotations for a particular class to the config file
 */
errResult stoSaveAnnotationsForClass(stoConfigFile &config, proClass *cls)
{
    if (!cls)
        return ER_Failure;

    // get or create the top section
    stoConfigFileSection *topSection = config.GetSection(proAnnoStr_title);
    if (!topSection)
    {
        return ER_Failure;
    }

    // create the section for this class
    string classSectionName("Class_");
    classSectionName += cls->GetName();

    // iterate through properties of class
    bool wroteTopSection = false;
    stoConfigFileSection* classSection = 0;
    for (int i=0; i != cls->GetPropertyCount(); i++)
    {
        proProperty *prop = cls->GetPropertyAt(i);

        string propertySectionName(classSectionName);
        propertySectionName += "_";
        propertySectionName += prop->GetName();

        // save the annotations for the property
        bool wroteClassSection = false;
        stoConfigFileSection *propertySection = 0;
        for (int j=0; j != prop->GetPropertyCount(); j++)
        {
            // write out the property into the proper section
            proProperty *anno = prop->GetPropertyAt(j);

            if (anno->GetName() == "IsNative" || 
                anno->GetName() == "Name" || 
                anno->GetName() == proAnnoStrObjectType ) // skip builtins
                continue;

            if ( !wroteTopSection )
            {
                // add to list of classes if not already there
                topSection->SetValue("Class", cls->GetName(), true, false);

                classSection = config.GetSection(classSectionName);
                if (!classSection)
                {
                    return ER_Failure;
                }

                wroteTopSection = true;
            }

            if ( !wroteClassSection )
            {
                // add section for each property
                classSection->SetValue("Property", prop->GetName(), true, false);

                propertySection = config.GetSection(propertySectionName);
                if ( !propertySection )
                {
                    return ER_Failure;
                }

                wroteClassSection = true;
            }

            propertySection->SetValue(anno->GetName(), anno->ToString(prop));
        }
    }

    return ER_Success;
}

/** 
 * save annotation data for all classes derived from className to the config file
 */
errResult stoSaveAllClasses(stoConfigFile &config, const string &className)
{
    vector<proClass*> classes;
    proClass *parent = proClassRegistry::ClassForName<proClass*>(className);
    if (parent)
    {
        proClassRegistry::GetClassesOfType(parent, classes, true);
        classes.push_back( parent ); // also want to save the parent
        vector<proClass*>::iterator it = classes.begin();
        for (; it != classes.end(); it++)
        {
            if ( !ISOK( stoSaveAnnotationsForClass(config, (*it) ) ) )
            {
                return ER_Failure;
            }
        }
        return ER_Success;
    }
    return ER_Failure;
}
