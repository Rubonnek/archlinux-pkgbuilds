#include <getopt.h>        // For parsing arguments
#include <stdlib.h>        // For the exit(int) function
#include <string.h>        // For the strlen functions
#include <stdio.h>         // For printing output to stdout
#include "xxtea.h"         // For encrypting/decrypting files

int main (int argc, char **argv) 
{

	// Configure the long getopt options:
	struct option options[] =
	{
		{ "encrypt", required_argument,	NULL, 'e' },
		{ "decrypt", required_argument,	NULL, 'd' },
		{ "input-file", required_argument,	NULL, 'i' },
		{ "output-file", required_argument,	NULL, 'o' },
		{ "passphrase", required_argument,	NULL, 'p' },
		{ "help",	no_argument,		NULL, 'h' },
		{ NULL, 0, 0, 0 }
	};

	// Needed variables:
	bool user_wants_to_encrypt_a_file = false;
	bool user_wants_to_decrypt_a_file = false;
	const char* input_file = 0;
	const char* output_file = 0;
	const char* passphrase = 0;
	char input_file_buffer[512];
	char output_file_buffer[512];
	char passphrase_buffer[512];

	int n = 0;
	while (n >= 0)
	{
		// getopt_long sets n < 0 once it's done fetching data.
		n = getopt_long(argc, argv, "dei:o:p:h", options, NULL);
		if (n < 0)
		{
			// Since n < 0, the while condition (n >= 0) is false and we exit the loop.
			// This continue exits the loop.
			continue;
		}

		switch (n)
		{
			case 'h':
				printf(
						"Description:\n"
						"\t%s is a program for encrypting/decrypting files using the xxtea library included with Cocos2D-X\n"
						"\n"
						"Usage:\n"
						"\n"
						"	-e, --encrypt\n"
						"	-d, --decrypt\n"
						"	-i, --input-file\n"
						"	-o, --output-file\n"
						"	-p, --passphrase\n"
						"\n"
						"Example:\n"
						"\n"
						"%s -e -i encrypted_file -o decrypted_file -p passphrase_used_when_file_was_encrypted\n", argv[0], argv[0]
					  );

				exit(0);
			case 'e':
				user_wants_to_encrypt_a_file = true;
				break;
			case 'd':
				user_wants_to_decrypt_a_file = true;
				break;
			case 'i':
				strncpy(input_file_buffer, optarg, sizeof input_file_buffer);
				input_file_buffer[(sizeof input_file_buffer) - 1] = '\0';
				input_file = input_file_buffer;
				//printf("input file is: %s\n", input_file);
				break;
			case 'o':
				strncpy(output_file_buffer, optarg, sizeof output_file_buffer);
				output_file_buffer[(sizeof output_file_buffer) - 1] = '\0';
				output_file = output_file_buffer;
				//printf("output file is: %s\n", output_file);
				break;
			case 'p':
				strncpy(passphrase_buffer, optarg, sizeof passphrase_buffer);
				passphrase_buffer[(sizeof passphrase_buffer) - 1] = '\0';
				passphrase = passphrase_buffer;
				//printf("passphrase is: %s\n", passphrase);
				break;
		}
	}

	// Error checking

	// Check that only one action flag was specified:
	if ( ( user_wants_to_encrypt_a_file && user_wants_to_decrypt_a_file ) ||   ( ( !user_wants_to_encrypt_a_file ) && ( !user_wants_to_decrypt_a_file ) ) ) 
	{
		fprintf(stderr, "Error: must specify one action only: either decryption (-d) or encryption (-e).\n");
		exit(1);
	}

	// Check that both, input file and output file were specified:
	if ( ( !input_file ) &&  ( !output_file ) )
	{
		// Then the user specified both
		fprintf(stderr, "Error: must only specify both: an input file and an output file\n");
		exit(2);
	}

	// Check that the passphrase was specified:
	if ( ! passphrase )
	{
		fprintf(stderr, "Error: the passphrase (-p) must be given. Otherwise encryption, nor decryption cannot process.\n");
		exit(3);
	}


	// By now we got all that we need. Check that we if we are about to encrypt like a boss:
	if ( user_wants_to_encrypt_a_file )
	{
		// Open the file handles:
		FILE * input_file_handle = fopen(input_file, "r");
		FILE * output_file_handle = fopen(output_file, "w");

		if (input_file_handle == NULL) {
			fprintf(stderr, "Can't open input file %s\n", input_file);
			exit(4);
		}

		if (output_file_handle == NULL) {
			fprintf(stderr, "Can't open output file %s\n", output_file);
			exit(5);
		}

		// Check how many bytes we got in the file:
		fseek (input_file_handle, 0, SEEK_END);
		size_t length = ftell (input_file_handle);

		// Allocate the bytes we previously found:
		char * buffer = (char *)malloc(length + 1);

		// Reset the handler back to the beginning:
		fseek (input_file_handle, 0, SEEK_SET);

		// Now slurp the file into the buffer:
		if (buffer)
		{
			fread (buffer, 1, length, input_file_handle);
		}

		// We can now close the input file stream since we got the contents in memory:
		fclose (input_file_handle);

		// And set the null character at the end of the string just in case (this is why we allocated length + 1 in the malloc above):
		buffer[length] = '\0';

		if (buffer)
		{
			// Debug:
			//printf("Let's see what we got in the contents:\n%s", buffer);

			// start to process your data / extract strings here...
			xxtea_long len;

			// Encrypt the file:
			unsigned char *encrypted_text = xxtea_encrypt((unsigned char*)buffer, length, (unsigned char*)passphrase, strlen(passphrase), &len);

			// Write the encrypted file to disk:
			fwrite(encrypted_text, sizeof(unsigned char), len, output_file_handle);

			// Close the output file handle, we are done writing:
			fclose(output_file_handle);

			// Free the encrypted text;
			free(encrypted_text);

			// Free the buffer contents now that we are done
			free(buffer);
		}

	}
	else if ( user_wants_to_decrypt_a_file )
	{

		// Open the file handles:
		FILE * input_file_handle = fopen(input_file, "r");
		FILE * output_file_handle = fopen(output_file, "w");

		if (input_file_handle == NULL) {
			fprintf(stderr, "Can't open input file %s\n", input_file);
			exit(4);
		}

		if (output_file_handle == NULL) {
			fprintf(stderr, "Can't open output file %s\n", output_file);
			exit(5);
		}

		// Check how many bytes we got in the file:
		fseek (input_file_handle, 0, SEEK_END);
		size_t length = ftell (input_file_handle);

		// Allocate the bytes we previously found:
		char * buffer = (char *)malloc(length + 1);

		// Reset the handler back to the beginning:
		fseek (input_file_handle, 0, SEEK_SET);

		// Now slurp the file into the buffer:
		if (buffer)
		{
			fread (buffer, 1, length, input_file_handle);
		}

		// We can now close the input file stream since we got the contents in memory:
		fclose (input_file_handle);

		// And set the null character at the end of the string just in case (this is why we allocated length + 1 in the malloc above):
		buffer[length] = '\0';

		if (buffer)
		{
			// Debug:
			//printf("Let's see what we got in the contents:\n%s", buffer);

			// Decrypt the file:
			xxtea_long len; // Variable for holding the length of the decrypted file:
			unsigned char *decrypted_text = xxtea_decrypt((unsigned char*)buffer, length, (unsigned char*)passphrase, strlen(passphrase), &len);

			// Write the decrypted file to disk:
			fwrite(decrypted_text, sizeof(unsigned char), len, output_file_handle);

			// Close the output file handle, we are done writing:
			fclose(output_file_handle);

			// Free the encrypted text;
			free(decrypted_text);

			// Free the buffer contents now that we are done
			free(buffer);
		}
	}

	return 0;
}
