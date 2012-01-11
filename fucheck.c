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
	}

	//xmlTextReaderClose(reader);
}

void getSetInfo(xmlDocPtr file) {
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	xmlChar *xpath = (xmlChar*) "/Set";

	context = xmlXPathNewContext(file);
	if (context == NULL) {
		fprintf(stderr, "Error in xmlXPathNewContext\n");
		exit(EXIT_FAILURE);
	}

	// Selecciona el elemento Set de la raìz del documento
	result = xmlXPathEvalExpression(xpath, context);
	if (result == NULL) {
		fprintf(stderr, "Error in xmlXPathEvalExpression\n");
		exit(EXIT_FAILURE);
	}

	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		xmlXPathFreeObject(result);
		fprintf(stderr, "No Set tag, invalid file.\n");
		exit(EXIT_FAILURE);
	}

	printf("%d\n", result->nodesetval->nodeNr);
	xmlXPathFreeObject(result);
}

xmlDocPtr getDoc(char *filename) {
	xmlDocPtr doc;
	doc = xmlParseFile(filename);

	if (doc == NULL) {
		fprintf(stderr, "No se pudo parsear el documento.\n");
		exit(EXIT_FAILURE);
	}

	return doc;
}

int main(int argc, char **argv) 
{
	char *docname;

	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
		return(0);
	}

	docname = argv[1];
	getSetInfo(getDoc(docname));
	//streamRtsFile(docname);

	return(1);
}
