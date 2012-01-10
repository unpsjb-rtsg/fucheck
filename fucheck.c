#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlreader.h>

void processNode(xmlTextReaderPtr reader) {
	xmlChar *name;
	name = xmlTextReaderLocalName(reader);
	printf("%s\n", name);
	xmlFree(name);
}

void streamRtsFile(char *file) 
{
	xmlTextReaderPtr reader;
	int ret;

	reader = xmlNewTextReaderFilename(file);

	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			processNode(reader);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			printf("%s: failed to parse\n", file);
		}
	} else {
		printf("Unable to open %s\n", file);
	}

	//xmlTextReaderClose(reader);
}

int main(int argc, char **argv) 
{
	char *docname;

	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
		return(0);
	}

	docname = argv[1];
	streamRtsFile(docname);

	return(1);
}
