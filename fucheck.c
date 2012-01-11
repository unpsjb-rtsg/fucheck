#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

int rtsCount;
int expectedRtsCount;

void processNode(xmlTextReaderPtr reader) {
	xmlChar *name;
	name = xmlTextReaderLocalName(reader);
	if (xmlStrcmp(name,  (const xmlChar*)"Set") == 0) { 
		printf("%s\n",  name);
	}
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
		xmlTextReaderClose(reader);
	}

}

void getSetInfo(xmlTextReaderPtr reader) {
	if (reader == NULL) {
		fprintf(stderr, "Unable to access the file.\n");
		exit(EXIT_FAILURE);
	}

	int ret;
	ret = xmlTextReaderRead(reader);

	// Si no se recupera un siguiente nodo, se termina la ejecución
	if (ret != 1) {
		fprintf(stderr, "ERROR: Unable to parse XML file.\n");
		exit(EXIT_FAILURE);
	}

	xmlChar *name;
	name = xmlTextReaderLocalName(reader);

	// Si el nodo recuperado no corresponde al tag Set, se aborta
	if (xmlStrcasecmp(name,  (const xmlChar*)"Set") != 0) { 
		xmlFree(name);
		fprintf(stderr, "ERROR: No `Set` tag. Invalid XML file.\n");
		exit(EXIT_FAILURE);
	}
	xmlFree(name);
	
	// Comprueba que el nodo contenga atributos
	if (xmlTextReaderHasAttributes(reader) != 1) {
		fprintf(stderr, "ERROR: `Set` tag has no attributes.\n");
		exit(EXIT_FAILURE);
	}

	xmlChar *value;

	value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"size");
	printf("Expected number of RTS: %s\n", value);
	xmlFree(value);

	value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"n");
	printf("Number of tasks per RTS: %s\n", value);
	xmlFree(value);
}

xmlTextReaderPtr getDoc(char* file) {
	xmlTextReaderPtr reader;

	reader = xmlNewTextReaderFilename(file);

	if (reader == NULL) {
		fprintf(stderr, "Unable to open %s\n", file);
		exit(EXIT_FAILURE);
	}

	return reader;
}

int main(int argc, char **argv) 
{
	char *docname;

	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
		return(0);
	}

	docname = argv[1];
	xmlTextReaderPtr reader = getDoc(docname);
	getSetInfo(reader);
	xmlFreeTextReader(reader);

	//streamRtsFile(docname);

	return(1);
}
