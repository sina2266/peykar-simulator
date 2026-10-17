/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Call clcache with the given arguments, except the first one, which is the
    // full path to the MSVC compiler.

    #define STRING_SIZE 32768

    char clcacheCommand[STRING_SIZE] = "clcache";
    int k = 6;

    for (int i = 2; i < argc; ++i) {
        clcacheCommand[++k] = ' ';

        for (int j = 0; argv[i][j]; ++j) {
            if (argv[i][j] == '\\') {
                clcacheCommand[++k] = '\\';
                clcacheCommand[++k] = '\\';
            } else {
                clcacheCommand[++k] = argv[i][j];
            }
        }
    }

    clcacheCommand[++k] = '\0';

    return system(clcacheCommand);
}
