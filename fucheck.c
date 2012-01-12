#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <gsl/gsl_statistics.h>

int rtsNr; 	// Número efectivo de STR en el archivo
int expRtsNr;	// Número esperado de STR en el archivo
int taskNr;	// Nùmero de tareas de cada STR
double fu;	// FU calculado para cada STR
double expFu;	// FU esperado para cada STR
double *fuArray;

void processNode(xmlTextReaderPtr reader) {
	xmlChar *name;
	xmlChar *wcet;
	xmlChar *period;
	xmlChar *expFuChar;

	name = xmlTextReaderLocalName(reader);
	if (xmlStrcasecmp(name,  (const xmlChar*)"S") == 0) { 
		// Tag de inicio
		if (xmlTextReaderNodeType(reader) == 1) {
			rtsNr = rtsNr + 1;
			
			// Comprueba que el nodo contenga atributos
			if (xmlTextReaderHasAttributes(reader) != 1) {
				fprintf(stderr, "ERROR: `S` tag has no attributes.\n");
				exit(EXIT_FAILURE);
			}
			
			expFuChar = xmlTextReaderGetAttribute(reader,  (const xmlChar*)"U");
			expFu = atof((char*) expFuChar) / 100.0;
			xmlFree(expFuChar);
		}

		// Tag de cierre
		if (xmlTextReaderNodeType(reader) == 15) {
			double diff;
			diff = abs(fu - expFu);
			if (diff > 0.05) {
				fprintf(stderr, "ERROR: %.3f <> %.3f\n", fu, expFu);
			}
			fuArray[rtsNr - 1] = fu;
			fu = 0.0;
		}
	}

	// Tarea
	if (xmlStrcasecmp(name,  (const xmlChar*)"i") == 0) { 
		wcet = xmlTextReaderGetAttribute(reader, (const xmlChar*)"C");
		period = xmlTextReaderGetAttribute(reader, (const xmlChar*)"T");
		fu = fu + atof((char*) wcet) / atof((char*) period);
		xmlFree(wcet);
		xmlFree(period);
	}

	xmlFree(name);
}

void streamRtsFile(xmlTextReaderPtr reader) 
{
	if (reader == NULL) {
		fprintf(stderr, "Unable to access the file.\n");
		exit(EXIT_FAILURE);
	}

	int ret;

	ret = xmlTextReaderRead(reader);
	while (ret == 1) {
		processNode(reader);
		ret = xmlTextReaderRead(reader);
	}

	if (ret != 0) {
		printf("Failed to parse.\n");
		return;
	}

	printf("Number of RTS in file: %d\n", rtsNr);

	printf("=== FU result ===\n");
	double mean = gsl_stats_mean(fuArray, 1, expRtsNr);
	printf("Mean: %.3f\n", mean);
	double variance = gsl_stats_variance_m(fuArray, 1, expRtsNr, mean);
	printf("Variance: %.3f\n", variance);
	double stddev = gsl_stats_sd_m(fuArray, 1, expRtsNr, mean);
	printf("Std. Dev: %.3f\n",  stddev);
	double max = gsl_stats_max(fuArray, 1, expRtsNr);
	printf("Max:  %.3f\n", max);
	double min = gsl_stats_min(fuArray, 1, expRtsNr);
	printf("Min: %.3f\n", min);
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

	printf("=== File info from header ===\n");

	xmlChar *value;

	// Número de grupos de tareas en el archivo
	value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"size");
	expRtsNr = atoi((char*) value);
	printf("Expected number of RTS: %d\n", expRtsNr);
	xmlFree(value);

	// Reservamos memoria para almacenar los FU
	fuArray = (double *) malloc(expRtsNr * sizeof(double));

	// Cantidad de tareas por cada sistema
	value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"n");
	taskNr = atoi((char*) value);
	printf("Number of tasks per RTS: %d\n", taskNr);
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
	rtsNr = 0;

	char *docname;

	if (argc <= 1) {
		printf("Usage: %s file\n", argv[0]);
		return(0);
	}

	docname = argv[1];

	xmlTextReaderPtr reader = getDoc(docname);
	getSetInfo(reader);
	streamRtsFile(reader);

	free(fuArray);
	xmlFreeTextReader(reader);

	return(1);
}
