int __getmainargs(int *, char ***, char ***, int, void *);
void __main(void) {}
int main(int, char **);
int mainCRTStartup(void) {
  int argc;
  char **argv;
  char **envp;
  __getmainargs(&argc, &argv, &envp, 0, 0);
  return main(argc, argv);
}
