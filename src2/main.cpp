#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

#if __has_include("pocketpy_c.h")
    #include "pocketpy_c.h"
#else
    #include "pocketpy.h"
#endif

#ifdef _WIN32

#include <windows.h>

std::string pkpy_platform_getline(bool* eof){
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    std::wstringstream wss;
    WCHAR buf;
    DWORD read;
    while (ReadConsoleW(hStdin, &buf, 1, &read, NULL) && buf != L'\n') {
        if(eof && buf == L'\x1A') *eof = true;  // Ctrl+Z
        wss << buf;
    }
    std::wstring wideInput = wss.str();
    int length = WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), NULL, 0, NULL, NULL);
    std::string output;
    output.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), &output[0], length, NULL, NULL);
    if(!output.empty() && output.back() == '\r') output.pop_back();
    return output;
}

#else

std::string pkpy_platform_getline(bool* eof){
    std::string output;
    if(!std::getline(std::cin, output)){
        if(eof) *eof = true;
    }
    return output;
}

#endif

static int f_input(pkpy_vm* vm){
    if(!pkpy_is_none(vm, -1)){
        pkpy_CString prompt;
        bool ok = pkpy_to_string(vm, -1, &prompt);
        if(!ok) return 0;
        std::cout << prompt << std::flush;
    }
    bool eof;
    std::string output = pkpy_platform_getline(&eof);
    pkpy_push_string(vm, pkpy_string(output.c_str()));
    return 1;
}

int main(){
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    pkpy_vm* vm = pkpy_new_vm(true);

    pkpy_push_function(vm, "input(prompt=None) -> str", f_input);
    pkpy_eval(vm, "__import__('builtins')");
    pkpy_setattr(vm, pkpy_name("input"));

    const char* src = R"(
try:
    print(0,' ', '', exp=' ')
except ValueError:
    pass

def test(a, b, c = " "):
    try:
        print(a)
        print(b, exp=" ")
        print(c)
    except:
        print(b, exp21=" ")
try:
    i = 1
    print(i)
    j = 2
    print(j)
    t = 3
    print(t)
    test(i, j)
except:
    i = 11
    print(i)
    j = 21
    print(j)
    t = 31
    print(t)
    print('xxx')
print('done')
)";
    const char* filename = "pydemo.py";
    char* out;
    bool ok;
    pkpy_compile_to_string(vm, src, filename, 0, &ok, &out);

    pkpy_exec(vm, src);
    //pkpy_compile_to_string(vm, )
    /*


    if(argc == 1){
        void* repl = pkpy_new_repl(vm);
        bool need_more_lines = false;
        while(true){
            std::cout << (need_more_lines ? "... " : ">>> ");
            bool eof = false;
            std::string line = pkpy_platform_getline(&eof);
            if(eof) break;
            need_more_lines = pkpy_repl_input(repl, line.c_str());
        }
        pkpy_delete_vm(vm);
        return 0;
    }
    
    if(argc == 2){
        std::string argv_1 = argv[1];
        if(argv_1 == "-h" || argv_1 == "--help") goto __HELP;

        std::filesystem::path filepath(argv[1]);
        filepath = std::filesystem::absolute(filepath);
        if(!std::filesystem::exists(filepath)){
            std::cerr << "File not found: " << argv_1 << std::endl;
            return 2;
        }        
        std::ifstream file(filepath);
        if(!file.is_open()){
            std::cerr << "Failed to open file: " << argv_1 << std::endl;
            return 3;
        }
        std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // set parent path as cwd
        std::filesystem::current_path(filepath.parent_path());

        bool ok = pkpy_exec_2(vm, src.c_str(), filepath.filename().string().c_str(), 0, NULL);
        if(!ok) pkpy_clear_error(vm, NULL);
        pkpy_delete_vm(vm);
        return ok ? 0 : 1;
    }
*/
__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}
