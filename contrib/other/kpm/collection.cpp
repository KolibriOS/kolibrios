#include "tinyxml/tinyxml.h"
#include "package.h"

// *INDENT-OFF*
const char *key_collection  = "collection";
const char *key_package     = "package";
const char *key_name        = "name";
const char *key_version     = "version";
const char *key_group       = "group";
const char *key_description = "description";
const char *key_title       = "title";
const char *key_release     = "release";
const char *key_file        = "file";
// *INDENT-ON*

int package_id;

collection_t *
load_collection_file(const char *name)
{
	TiXmlDocument doc;
	TiXmlElement *col;
	collection_t *collection = NULL;

	doc.LoadFile(name);
	col = doc.FirstChildElement(key_collection);
	if (col)
	{
		TiXmlElement *xmlpkg;
		TiXmlElement *xmle;

		collection = (collection_t *) malloc(sizeof(collection_t));
		INIT_LIST_HEAD(&collection->packages);

		xmlpkg = col->FirstChildElement(key_package);

		while (xmlpkg)
		{
			package_t *pkg;

			pkg = (package_t *) malloc(sizeof(package_t));

			INIT_LIST_HEAD(&pkg->file_list);

			pkg->id = package_id++;
			pkg->name = strdup(xmlpkg->Attribute(key_name));
			pkg->version = strdup(xmlpkg->Attribute(key_version));
			pkg->group = strdup(xmlpkg->Attribute(key_group));

			xmle = xmlpkg->FirstChildElement(key_description);
			pkg->description = strdup(xmle->Attribute(key_title));

			xmle = xmlpkg->FirstChildElement(key_release);
			pkg->filename = strdup(xmle->Attribute(key_file));

			list_add_tail(&pkg->list, &collection->packages);
			xmlpkg = xmlpkg->NextSiblingElement();
		};
    };

    return collection;
};
