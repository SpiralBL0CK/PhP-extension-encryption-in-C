// include the PHP API itself
#include <php.h>
// then include the header of your extension
#include "php_heloworld.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>


void HexDumpMemory(void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04X ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02X", pc[i]);
		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		}
		else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}



void XorBlock(DWORD dwStartAddress, DWORD dwSize)
{
	__asm
	{
        push eax
        push ecx
        mov ecx, dwStartAddress          // Move Start Address to ECX
        add ecx, dwSize                  // Add the size of the function to ECX
        mov eax, dwStartAddress          // Copy the Start Address to EAX

        crypt_loop:                         // Start of the loop
            xor byte ptr ds:[eax], 0x4D     // XOR The current byte with 0x4D
            inc eax                         // Increment EAX with dwStartAddress++
            cmp eax,ecx                     // Check if every byte is XORed
        jl crypt_loop;                      // Else jump back to the start label

        pop ecx // pop ECX from stack
        pop eax // pop EAX from stack
    }

}

void TestFunction()
{
	printf("%d",10);

}

// Marks the end of testfunction()
void FunctionStub() { return; }

DWORD GetFuncSize(DWORD* Function, DWORD* StubFunction)
{
	
	DWORD dwFunctionSize = 0, dwOldProtect;
	DWORD *fnA = NULL, *fnB = NULL;

	fnA = (DWORD *)Function;
	fnB = (DWORD *)StubFunction;
	dwFunctionSize = (fnB - fnA);
	VirtualProtect(fnA, dwFunctionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect); // Need to modify our privileges to the memory
	
	return dwFunctionSize;
}

// register our function to the PHP API 
// so that PHP knows, which functions are in this module
zend_function_entry helloworld_php_functions[] = {
    PHP_FE(helloworld_php, NULL)
    {NULL, NULL, NULL}
};

// some pieces of information about our module
zend_module_entry helloworld_php_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_HELLOWORLD_EXTNAME,
    helloworld_php_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_HELLOWORLD_VERSION,
    STANDARD_MODULE_PROPERTIES
};

// use a macro to output additional C code, to make ext dynamically loadable
ZEND_GET_MODULE(helloworld_php)

// Finally, we implement our "Hello World" function
// this function will be made available to PHP
// and prints to PHP stdout using printf
PHP_FUNCTION(helloworld_php) {
    php_printf("Hello World! (from our extension)\n");
    DWORD dwFuncSize = GetFuncSize((DWORD*)&TestFunction, (DWORD*)&FunctionStub);
    php_printf("\n\nEncrypted:\n");
    XorBlock((DWORD)&TestFunction, dwFuncSize); // XOR encrypt the function
    HexDumpMemory(&TestFunction, dwFuncSize);
}


