#define ING_STRIP_PREFIX

#define ING_CORE_IMPL
#define ING_IMPL

#include "../ing_cli.h"

Ing_Define_Config(Arg_Config);

Ing_Define_Keyword(Help_Long, "help", &Arg_Config);
Ing_Define_Keyword(Help_Short, "h", &Arg_Config);
Ing_Define_Keyword(Interactive_Long, "interactive", &Arg_Config);
Ing_Define_Keyword(Interactive_Short, "i", &Arg_Config);
Ing_Define_Keyword(Build_Long, "build", &Arg_Config);
Ing_Define_Keyword(Build_Short, "b", &Arg_Config);
Ing_Define_Keyword(Run_Long, "run", &Arg_Config);
Ing_Define_Keyword(Run_Long, "r", &Arg_Config);
Ing_Define_Keyword(Repl_Short, "repl", &Arg_Config);


i32 main(i32 argc, Ing_String argv[])
{
    Ing_Arg_Parser ap = ing_arg_parser_make(argv, &Arg_Config);

    ing_ap_persistent_flags(&ap, 
        ing_maybe_flag(Ing_Help_Long, Ing_Help_Short),
        ing_maybe_flag(Ing_Interactive_Long, Ing_Interactive_Short),
    );

    Ing_AP_Result res = ing_ap_oneof(&ap, 
        ing_maybe_cmd(Ing_Build_Long, Ing_Build_Short)
        ing_maybe_cmd(Ing_Run_Long, Ing_Run_Short),
        ing_maybe_cmd(Ing_Repl_Long, Ing_Repl_Short)
   );

    if (res.kind == Ing_Err)
    {
        ing_error("invalid argument: %s, expected: %s", res)
        return 1;
    }

    switch (cmd.val.ok)
    {
        case Ing_Build_Long: case Ing_Build_Short   : ing_build();
        case Ing_Run_Long: case Ing_Run_Short       : ing_run();
        case Ing_Repl_Long: case Ing_Repl_Short     : ing_repl();
    }

    return 0;
}
