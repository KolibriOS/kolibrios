#include "tinyxml/tinyxml.h"
#include "package.h"

const char *key_collection  = "collection";
const char *key_package     = "package";
const char *key_name        = "name";
const char *key_version     = "version";
const char *key_description = "description";
const char *key_title       = "title";
const char *key_release     = "release";
const char *key_file        = "file";

int package_id;

void parse_group(pkg_group_t* gr, TiXmlElement *xmlgroup)
{
	TiXmlElement *xmlpkg;
	TiXmlElement *xmle;

    xmlpkg = xmlgroup->FirstChildElement(key_package);
	while (xmlpkg)
	{
		package_t *pkg;

		pkg = (package_t*)malloc(sizeof(package_t));

        INIT_LIST_HEAD(&pkg->file_list);
		pkg->id = package_id++;
        pkg->name = strdup(xmlpkg->Attribute(key_name));
        pkg->version = strdup(xmlpkg->Attribute(key_version));

        xmle = xmlpkg->FirstChildElement(key_description);
        pkg->description = strdup(xmle->Attribute(key_title));

        xmle = xmlpkg->FirstChildElement(key_release);
        pkg->filename = strdup(xmle->Attribute(key_file));

        list_add_tail(&pkg->list, &gr->packages);
		xmlpkg = xmlpkg->NextSiblingElement();
	};
};


collection_t* load_collection_file(const char *name)
{
	TiXmlDocument doc;
	TiXmlElement *col;
    collection_t *collection = NULL;

	doc.LoadFile(name);
    col = doc.FirstChildElement(key_collection);
	if (col)
	{
        collection = (collection_t*)malloc(sizeof(collection_t));
        INIT_LIST_HEAD(&collection->groups);

		TiXmlElement* xmlgroup = col->FirstChildElement();
		if (xmlgroup)
		{
			pkg_group_t *gr;

			gr = (pkg_group_t*)malloc(sizeof(pkg_group_t));
			INIT_LIST_HEAD(&gr->list);
			INIT_LIST_HEAD(&gr->packages);

            gr->name = strdup(xmlgroup->Value());
            list_add_tail(&gr->list, &collection->groups);
			parse_group(gr, xmlgroup);
		};
	};

	return collection;
}


