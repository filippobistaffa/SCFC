#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "scfc.h"

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

size_t write_xml_output(agent *root, size_t users, char *filename);

#endif /* OUTPUT_H_ */
