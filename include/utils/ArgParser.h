#ifndef ARG_PARSER_H
#define ARG_PARSER_H

class ArgParser
{
public:
    static void log(int argc, char** argv);
    static bool find(int argc, char** argv, const char* opt);
    static char* getArg(int argc, char** argv, const char* ext);

    //bool start_scantra_;
    //bool open_project_;
    //bool vulkan_validation_;
    //bool select_device_;
    //std::filesystem::path project_path_;
};



#endif