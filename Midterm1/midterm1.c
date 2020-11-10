#include <stdio.h>

typedef struct
{
    int index;
    char** lines;
    int size;

} FILES;

void print(char *p[], int n)
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf("Path%d: %s\n", i, p[i]);
    }
}
void print_FILES(FILES* p, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for(int j = 0; j < p[i].size; ++j)
        {
            printf("FILE%d: %s\n", p[i].index, p[i].lines[j]);
        }
    }
}



int main(int argc, char *argv[]) 
{
    int n;
    int c;
    FILE *input_file, *output_file;
    FILE *temp_file;  
    char line[BUFSIZ];


    char **array = NULL;
    int nlines = 10;

    //Store paths
  
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s from-file to-file\n", *argv);
        exit(1);
    }
    if ((input_file = fopen(argv[1], "r")) == NULL) {
        perror(argv[1]);
        exit(1);
    }
    if ((output_file = fopen(argv[2], "w+")) == NULL) {
        perror(argv[2]);
        exit(1);
    }

    array = malloc(nlines * sizeof(*array));
    if (!array)
    {
        perror("malloc");
        exit(1);
    }

    int i = 0;
    while (fgets(line, sizeof(line), input_file))
    {

        int l = strlen(line);

        array[i] = malloc(l);
        if (!array[i])
        {
            perror("malloc");
            exit(1);
        }
        
        strncpy(array[i], line, l-1);
        array[i][l-1] = '\0';
        i += 1;

        if (i >= nlines)
        {
            nlines = nlines * 2;
            array = realloc(array, nlines * sizeof(*array));

            if (!array)
            {
                perror("malloc");
                exit(1);
            }
        }
    }

    FILES* input_files = malloc(i * sizeof(FILES));

    if (!input_files)
    {
        perror("malloc");
        exit(1);
    }

    for(int j = 0; j < i; ++j)
    {
        if ((temp_file = fopen(array[j], "r")) == NULL) {
            perror(temp_file);
            exit(1);
        }
        input_files[j].index = j;

        input_files[j].lines = malloc(nlines * sizeof(*array));

        if (!input_files[j].lines)
        {
            perror("malloc");
            exit(1);
        }
        int k = 0;
        while (fgets(line, sizeof(line), temp_file) != NULL) 
        {
            int l = strlen(line);

            input_files[j].lines[k] = malloc(l);
            if (!input_files[j].lines[k])
            {
                perror("malloc");
                exit(1);
            }
            
            strncpy(input_files[j].lines[k], line, l-1);
            input_files[j].lines[k][l-1] = '\0';
            k += 1;

            //REALLOCATE    
            if (k >= nlines)
            {
                nlines = nlines * 2;
                input_files[j].lines[k] = realloc(input_files[j].lines[k], nlines * sizeof(*array));

                if (!input_files[j].lines[k])
                {
                    perror("malloc");
                    exit(1);
                }
            }
            
        }
        input_files[j].size = k;
        
        close(temp_file);
    }


    //COPY
    int next_file = 1;
    int next_line = 0;
    for(int j = 0; j < i; ++j)
    {
        for(int k = 0; k < input_files[j].size; ++k)
        {
            int insert = 0;
            while(next_file < i)
            {
                while(next_line < input_files[next_file].size)
                {
                    if (strcmp(input_files[j].lines[k], input_files[next_file].lines[next_line]) == 0)
                    {
                        insert = 1;
                    }
                    ++next_line;
                }
                ++next_file;
                if(next_file == j) ++next_file;
            }
            next_file = 0;
            next_line = 0;
            if(insert == 0) fprintf(output_file, "%s %s\n", input_files[j].lines[k], array[j]);
        }
       
    }



    print_FILES(input_files, i);

    for(int j = 0; j < i; ++j)
    {
        free(array[j]);
    }
    for(int j = 0; j < i; ++j)
    {
        for(int k = 0; k < input_files[j].size; ++k)
        {
            free(input_files[j].lines[k]);
        }
        free(input_files[j].lines);
    }



    free(array);
    free(input_files);

    fclose(input_file);
    fclose(output_file);
    return 0;
}