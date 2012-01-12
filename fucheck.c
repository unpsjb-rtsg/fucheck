#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <gsl/gsl_statistics.h>

int rtsNr; 	// Número efectivo de STR en el archivo
int expRtsNr;	// Número esperado de STR en el archivo
int taskNr;	// Nùmero de tareas de cada STR
double fu;	// FU calculado para cada STR
double expFu;	// FU esperado para cada STR
double gexpFu;	// FU esperado global
double *fuArray;	// Arreglo con los FU de cada STR
double delta;	// Delta para comparar doubles
const char *progName;	// Nombre del programa

void processNode(xmlTextReaderPtr reader) 
{
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
			diff = fabs(fu - expFu);
			if (diff > delta) {
				fprintf(stderr, "ERROR -- RTS %d, wrong FU: %.3f <> %.3f\n", rtsNr, fu, expFu);
			}
			diff = fabs(fu - gexpFu / 100.0);
			if (diff > delta) {
				fprintf(stderr, "ERROR -- RTS %d, wrong FU: %.3f, expected %.3f\n", rtsNr, fu, gexpFu / 100.0); 
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

	printf("=== Analyzing ===\n");
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
	printf("Mean:\t\t%.3f\n", mean);
	double variance = gsl_stats_variance_m(fuArray, 1, expRtsNr, mean);
	printf("Variance:\t%.3f\n", variance);
	double stddev = gsl_stats_sd_m(fuArray, 1, expRtsNr, mean);
	printf("Std. Dev:\t%.3f\n",  stddev);
	double max = gsl_stats_max(fuArray, 1, expRtsNr);
	printf("Max:\t\t%.3f\n", max);
	double min = gsl_stats_min(fuArray, 1, expRtsNr);
	printf("Min:\t\t%.3f\n", min);
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

	printf("=== File info ===\n");

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

	// FU para todos los RTS
	if (gexpFu == 0) {
		value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"u");
		if (value == NULL) {
			xmlFree(value);
			fprintf(stderr, "ERROR: `Set` tag has no FU attribute. Specify one with --fu option.\n");
			exit(EXIT_FAILURE);
		}
		gexpFu = atof((char*) value);
		xmlFree(value);
	}
	printf("Expected FU for all RTS: %.3f (%.3f)\n", gexpFu, gexpFu / 100.0);

	printf("Delta: %.3f\n", delta);
}

xmlTextReaderPtr getDoc(char *file) 
{
	xmlTextReaderPtr reader;

	reader = xmlNewTextReaderFilename(file);

	if (reader == NULL) {
		fprintf(stderr, "Unable to open %s\n", file);
		exit(EXIT_FAILURE);
	}

	return reader;
}

void printUsage(FILE *stream,  int exitCode) 
{
	fprintf(stream, "Usage: %s [options] file\n", progName);
	fprintf(stream, 
			"\t-h  --help\tDisplay this usage information.\n"
			"\t-u  --fu\tExpected FU for all RTS in file.\n"
			"\t-d  --delta\tMaximum tolerance for FU values.\n");
	exit(exitCode);
}

void setGlobalExpFu(double fu)
{
	if (fu <= 0 || fu > 100) {
		fprintf(stderr, "Invalid FU: %.3f (%.3f)\n", fu, fu / 100.0);
		exit(EXIT_FAILURE);
	}
	gexpFu = fu;
}

void setDelta(double d) 
{
	if (d <= 0) {
		fprintf(stderr, "Invalid delta: %.3f\n", d);
		exit(EXIT_FAILURE);
	}
	delta = d;
}

int main(int argc, char **argv) 
{
	rtsNr = 0;

	char *docname;
	int nextOption;

	// Opciones validas, formato corto
	const char *shortOpts = "hu:d:";
	// Opciones en formato largo
	const struct option longOpts[] = {
		{"help", 0, NULL, 'h'}, 
		{"fu", 1, NULL, 'u'}, 
		{"delta", 1, NULL, 'd'}, 
		{NULL, 0, NULL, 0}
	};

	progName = argv[0];

	if (argc <= 1) {
		printUsage(stderr, EXIT_FAILURE);
	}

	delta = 0.005;

	do {
		nextOption = getopt_long(argc, argv, shortOpts, longOpts, NULL);

		switch (nextOption) {
			case 'h': // -h o --help
				printUsage(stdout, EXIT_SUCCESS);
			case 'u': // -u o --fu
				setGlobalExpFu(atof(optarg));
				break;
			case 'd': // -d o --delta
				setDelta(atof(optarg));
				break;
			case '?': // opcion invalida
				printUsage(stderr, EXIT_FAILURE);
			case -1: // no ha más opciones
				break;
			default: // inesperado
				abort();
		}
	} while (nextOption != -1);

	docname = argv[optind];
	
	printf("Analize file %s\n", docname);

	xmlTextReaderPtr reader = getDoc(docname);
	getSetInfo(reader);
	streamRtsFile(reader);

	free(fuArray);
	xmlFreeTextReader(reader);

	return(EXIT_SUCCESS);
}
