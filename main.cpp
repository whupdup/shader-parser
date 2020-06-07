#include <cstdio>

#include <engine/core/util.hpp>

#include "shader-parser.hpp"

void printLayoutInfo(const ArrayList<ShaderInfo::Layout>& layoutInfo);

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s shader file\n", argv[0]);
		return 1;
	}

	StringStream fileStream;
	Util::loadFileWithLinking(fileStream, argv[1], "#include");

	ShaderInfo shaderInfo;

	if (shaderInfo.parse(fileStream)) {
		printLayoutInfo(shaderInfo.getLayoutInfo());
	}

    return 0;
}

void printLayoutInfo(const ArrayList<ShaderInfo::Layout>& layoutInfo) {
	for (const auto& li : layoutInfo) {
		puts("LAYOUT INFO:");
		printf("\tLAYOUT TYPE: %s\n", ShaderInfo::stringifyLayoutType(li.type));
		
		puts("\tMEMORY QUALIFIERS:");

		for (const auto& mq : li.memoryQualifiers) {
			printf("\t\t%s\n", mq.c_str());
		}

		printf("\tTYPE QUALIFIER: %s\n", li.typeQualifier.c_str());
		printf("\tVARIABLE NAME: %s\n", li.name.c_str());
		puts("\tOPTIONS:");

		for (const auto& pair : li.options) {
			printf("\t\t%s = %d\n", pair.first.c_str(), pair.second);
		}

		puts("\tVARIABLES:");

		for (const auto& var : li.body) {
			if (var.isArray) {
				if (var.arraySize == -1) {
					printf("\t\t%s: %s[]\n", var.name.c_str(), var.typeName.c_str());
				}
				else {
					printf("\t\t%s: %s[%d]\n", var.name.c_str(), var.typeName.c_str(),
							var.arraySize);
				}
			}
			else {
				printf("\t\t%s: %s\n", var.name.c_str(), var.typeName.c_str());
			}
		}
	}
}
