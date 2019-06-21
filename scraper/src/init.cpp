#include <stdio.h> // for fprintf
#include <iostream> // for std::cout
#include <string.h> // for memcpy

#ifdef _WIN32
# include <windows.h>
#endif


char* REDDIT_AUTH[6];
char* AUTH_PTR;


void ef_reed(){
    char c;
    while((c = fgetc(stdin))){
        if (c != '\n'){
            *AUTH_PTR = c;
            ++AUTH_PTR;
        } else return;
    }
}


int main(){
    static const char* env_var = "RSCRAPER_REDDIT_CFG";
    
    char* buf = (char*)malloc(4096 * 2);
    
    std::cout << "* Reddit Configuration *" << std::endl;
    
    std::cout << "Absolute file path to save the config file to (will NOT create folders/directories for you): ";
    
    char* cfg_pth = buf;
    {
    char c;
    for(auto i = 0;  (c = fgetc(stdin));  ++i){
        if (c != '\n')
            *(buf++) = c;
        else break;
    }
    }
    *(buf++) = 0;
    
    char* auth = buf;
    memcpy(auth, "USERNAME: ", 10);
    char* AUTH_PTR_ENDS[6];
    AUTH_PTR = auth + 10;
    REDDIT_AUTH[0] = AUTH_PTR;
    auto i = 0;
    
    std::cout << "Username: ";
    ef_reed();
    AUTH_PTR_ENDS[i] = AUTH_PTR;
    memcpy(AUTH_PTR, "\nPASSWORD: ", 11);
    AUTH_PTR += 11;
    REDDIT_AUTH[++i] = AUTH_PTR;
    
    std::cout << "Password: ";
    ef_reed();
    AUTH_PTR_ENDS[i] = AUTH_PTR;
    memcpy(AUTH_PTR, "\nKEY_SCRT: ", 11);
    AUTH_PTR += 11;
    REDDIT_AUTH[++i] = AUTH_PTR;
    
    std::cout << "App Key (on reddit.com under preferences): ";
    ef_reed();
    *(AUTH_PTR++) = ':';
    std::cout << "App Secret (on reddit.com under preferences): ";
    ef_reed();
    AUTH_PTR_ENDS[i] = AUTH_PTR;
    memcpy(AUTH_PTR, "\nUSERAGNT: ", 11);
    AUTH_PTR += 11;
    REDDIT_AUTH[++i] = AUTH_PTR;
    
    std::cout << "User Agent: ";
    ef_reed();
    AUTH_PTR_ENDS[i] = AUTH_PTR;
    memcpy(AUTH_PTR, "\nPROXYURL: ", 11);
    AUTH_PTR += 11;
    REDDIT_AUTH[++i] = AUTH_PTR;
    
    std::cout << "Proxy url (or - if none): ";
    ef_reed();
    AUTH_PTR_ENDS[i] = AUTH_PTR;
    *AUTH_PTR = '\n'; // Terminate port number calculation
    
    FILE* cfg = fopen(cfg_pth, "wb");
    fwrite(auth,  1,  (uintptr_t)AUTH_PTR + 1 - (uintptr_t)auth,  cfg);
    fclose(cfg);
    
#ifdef _WIN32
    // Add environmental variables to the registry
    // TODO: Add to HKEY_USERS (i.e. just the user's environment) rather than HKEY_LOCAL_MACHINE
    
    HKEY hkey;
    LPCTSTR key_path = TEXT("HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Environment");
    LSTATUS l_status;
    l_status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_path, 0, KEY_ALL_ACCESS, &hkey);
    if (l_status != ERROR_SUCCESS){
        fprintf(stderr, "Cannot open registry to insert environmental variables\n");
        goto goto_diy_env;
    }
    l_status = RegSetValueEx(hkey,  env_var,  0,  REG_SZ,  (const BYTE*)cfg_pth,  strlen(cfg_pth) + 1);
    RegCloseKey(hkey);
    if (l_status != ERROR_SUCCESS){
        fprintf(stderr, "Cannot set environmental variables in registry\n");
        goto goto_diy_env;
    }
#endif
    
    goto_diy_env:
#ifdef _WIN32
    std::cout << "You need to add the following environmental variable using My Computer > Properties > Advanced > Environmental Variables:" << std::endl;
    std::cout << "Name\tValue" << std::endl;
    std::cout << env_var << "\t" << cfg_pth << std::endl;
#else
    std::cout << "You need to add the following line to your shell profile (such as ~/.bashrc):" << std::endl;
    std::cout << "export " << env_var << "=" << cfg_pth << std::endl;
#endif
}
