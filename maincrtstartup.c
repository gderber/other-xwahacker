int __getmainargs(int *, char ***, char ***, int, void *);
void __main(void) {}
int main(int, char **);
int mainCRTStartup(void) {
  int argc;
  char **argv;
  char **envp;
  // WinXP crashes if startupinfo pointer is NULL, so we need to provide one
  // From Win 7 on at least (possibly Vista even) this is not necessary.
  _startupinfo start_info;
  memset(&start_info, 0, sizeof(start_info));
  __getmainargs(&argc, &argv, &envp, 0, &start_info);
  return main(argc, argv);
}
