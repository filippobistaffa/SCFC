#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "scfc.h"

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE scf SYSTEM \"http://dl.dropbox.com/u/48064647/output.dtd\">\n"
#define INDENT(I) {for (i = 0; i < a->level * 2 + 2 + I; i++) fprintf(f, "\t");}

size_t write_xml_output(agent *root, size_t users, char *filename);

#endif /* OUTPUT_H_ */
