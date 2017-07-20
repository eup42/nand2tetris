/*
 * vmtranslator.c
 */

#include "parser.h"
#include "code_writer.h"

int main(int argc, char **argv)
{
    Parser parser = newParser();
    CodeWriter code_writer = newCodeWriter();

    if (argc != 2) {
        printf("Error: argument is invalid\n");
        return 1;
    }

    parser.init(&parser, argv[1]);
    code_writer.init(&code_writer, "test");

    code_writer.setFileName(&code_writer, argv[1]);

    while (parser.hasMoreCommands(&parser)) {
        parser.advance(&parser);

        switch(parser.commandType(&parser)) {
            case C_ARITHMETRIC:
                code_writer.writeArithmetric(&code_writer, parser.arg1(&parser));
                break;
            case C_PUSH:
                code_writer.writePushPop(&code_writer, C_PUSH, parser.arg1(&parser), parser.arg2(&parser));
                break;
            default:
                break;
        }
    }

    code_writer.close(&code_writer);

    parser.del(&parser);
    code_writer.del(&code_writer);

    return 0;
}
