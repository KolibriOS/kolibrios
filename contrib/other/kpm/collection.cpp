#include "tinyxml/tinyxml.h"
#include "collection.h"
#include <sys/kos_io.h>

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

int build_install_list(list_t *list, collection_t *collection)
{
    pkg_group_t *gr;
    int count = 0;

    list_for_each_entry(gr, &collection->groups, list)
    {
        package_t   *pkg, *tmp;

        list_for_each_entry(tmp, &gr->packages, list)
        {
            pkg = (package_t*)malloc(sizeof(package_t));

            pkg->id       = tmp->id;
            pkg->name     = strdup(tmp->name);
            pkg->version  = strdup(tmp->version);
            pkg->filename = strdup(tmp->filename);
            pkg->description = strdup(tmp->description);
            list_add_tail(&pkg->list, list);
//            printf("add package %s-%s\n", pkg->name, pkg->version);

            count++;
        }
    };
    return count;
}

int build_download_list(list_t *download, list_t *src)
{
    int count = 0;
    char *cache_path;
    package_t   *pkg, *tmp;
    fileinfo_t  info;
    list_for_each_entry(tmp, src, list)
    {
        cache_path = make_cache_path(tmp->filename);

        if( get_fileinfo(cache_path, &info) != 0)
        {
            pkg = (package_t*)malloc(sizeof(package_t));

            pkg->id       = tmp->id;
            pkg->name     = strdup(tmp->name);
            pkg->version  = strdup(tmp->version);
            pkg->filename = strdup(tmp->filename);
            pkg->description = strdup(tmp->description);
            list_add_tail(&pkg->list, download);
            count++;
            printf("add package %s-%s\n", pkg->name, pkg->version);
        };
    }
    return count;
};
