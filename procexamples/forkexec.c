main() 
{ 
    int return_value;
 
     printf("Forking process$\n");
     return_value=fork();
     printf("The process id is %d and return value is %d\n", getpid(), return_value);
     execl("/bin/ls","ls","-l",0);
     printf("This line is not printed$\n");
}


