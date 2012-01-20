#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <gsl/gsl_statistics.h>

#define DFLT_DELTA 0.005

int rtsNr; 	// Número efectivo de STR en el archivo
int expRtsNr;	// Número esperado de STR en el archivo
int taskNr;	// Número de tareas de cada STR
int validFuCnt;	// Número de STR con FU esperado
int invalidFuCnt;	// Número de STR con FU erroneo
int verbose;	// Presentar o no resultados individuales
int limit;	// Número de STR a verificar
int cont;	// Contador número de STR a verificar
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
			int flag;
			double diff, diff2;
			flag = 0;
			diff = fabs(fu - expFu);
			diff2 = fabs(fu - gexpFu / 100.0);
			if (diff > delta) {
				if (verbose) {
					fprintf(stderr, "ERROR -- RTS %d, wrong FU: %.5f, expected %.5f (S)\n", 
							rtsNr, fu, expFu);
				}
				invalidFuCnt = invalidFuCnt + 1;
				flag = 1;
			}
			else if (diff2 > delta) {
				if (verbose) {
					fprintf(stderr, "ERROR -- RTS %d, wrong FU: %.5f, expected %.5f (Set)\n", 
							rtsNr, fu, gexpFu / 100.0); 
				}
				// No incrementa el contador si ya se detecto como erroneo
				if (flag == 0) {
					invalidFuCnt = invalidFuCnt + 1;
				}
			}
			else {
				// FU correcto
				validFuCnt = validFuCnt + 1;
			}
			fuArray[rtsNr - 1] = fu;
			fu = 0.0;
			flag = 0;
			
			cont = cont - 1;
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

	if (verbose) {
		printf("=== Analyzing ===\n");
	}

	ret = xmlTextReaderRead(reader);
	while (ret == 1 && cont > 0) {
		processNode(reader);
		ret = xmlTextReaderRead(reader);
	}

	if (ret != 0 && cont > 0) {
		printf("Failed to parse.\n");
		return;
	}

	printf("=== FU result ===\n");
	double mean = gsl_stats_mean(fuArray, 1, limit);
	printf("Mean:\t\t%.5f (%.3f)\n", mean, mean * 100.0);
	double variance = gsl_stats_variance_m(fuArray, 1, limit, mean);
	printf("Variance:\t%.5f\n", variance);
	double stddev = gsl_stats_sd_m(fuArray, 1, limit, mean);
	printf("Std. Dev:\t%.5f\n",  stddev);
	double max = gsl_stats_max(fuArray, 1, limit);
	printf("Max:\t\t%.5f (%.3f)\n", max,  max * 100.0);
	double min = gsl_stats_min(fuArray, 1, limit);
	printf("Min:\t\t%.5f (%.3f)\n", min,  min * 100.0);
	printf("Total RTS:\t%d\n", rtsNr);
	printf("Valid RTS:\t%d\n", validFuCnt);
	printf("Invalid RTS:\t%d\n", invalidFuCnt);
}

void getSetInfo(xmlTextReaderPtr reader) 
{
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

	// Número de STR a verificar si no se paso opción --limit
	if (cont == 0) {
		cont = expRtsNr;
	}
	limit = cont;

	// Reservamos memoria para almacenar los FU
	fuArray = (double *) malloc(cont * sizeof(double));

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
			fprintf(stderr, "ERROR: `Set` tag has no FU value. Specify one with --fu option.\n");
			exit(EXIT_FAILURE);
		}
		gexpFu = atof((char*) value);
		xmlFree(value);
	}
	printf("Expected FU for all RTS: %.5f (%.3f)\n", gexpFu / 100.0, gexpFu);
	
	printf("RTS to analize: %d\n", cont);

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
			"\t-v  --verbose\tDisplay individiual RTS info.\n" 
			"\t-h  --help\tDisplay this usage information.\n"
			"\t-u  --fu\tExpected FU for all RTS in file.\n"
			"\t-d  --delta\tMaximum tolerance for FU values.\n"
			"\t-l  --limit\tTest first n RTS in file.\n");
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

void setLimit(int l)
{
	if (l <= 0) {
		fprintf(stderr, "Invalid limit: %d\n", l);
		exit(EXIT_FAILURE); 
	}
	cont = l;
}

int main(int argc, char **argv) 
{
	rtsNr = 0;
	validFuCnt = 0;
	invalidFuCnt = 0;

	char *docname;
	int nextOption;

	// Opciones validas, formato corto
	const char *shortOpts = "vhu:d:l:";
	// Opciones en formato largo
	const struct option longOpts[] = {
		{"verbose", 0, NULL, 'v'}, 
		{"help", 0, NULL, 'h'}, 
		{"fu", 1, NULL, 'u'}, 
		{"delta", 1, NULL, 'd'},
		{"limit", 1, NULL, 'l'}, 
		{NULL, 0, NULL, 0}
	};

	progName = argv[0];

	if (argc <= 1) {
		printUsage(stderr, EXIT_FAILURE);
	}

	delta = DFLT_DELTA;
	verbose = 0;
	cont = 0;

	do {
		nextOption = getopt_long(argc, argv, shortOpts, longOpts, NULL);

		switch (nextOption) {
			case 'v': // -s o --verbose
				verbose = 1;
				break;
			case 'h': // -h o --help
				printUsage(stdout, EXIT_SUCCESS);
			case 'u': // -u o --fu
				setGlobalExpFu(atof(optarg));
				break;
			case 'd': // -d o --delta
				setDelta(atof(optarg));
				break;
			case 'l': // -l o --limit
				setLimit(atoi(optarg));
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

	xmlTextReaderPtr reader = getDoc(docname);
	getSetInfo(reader);
	streamRtsFile(reader);

	free(fuArray);
	xmlFreeTextReader(reader);

	return(EXIT_SUCCESS);
}
