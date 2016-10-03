//
//  raycaster.c
//  CS430 Project 2
//
//  Frankie Berry
//

// pre-processor directives
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


// function prototypes
int next_c(FILE* json);

void expect_c(FILE* json, int d);

void skip_ws(FILE* json);

char* next_string(FILE* json);

double next_number(FILE* json);

double* next_vector(FILE* json);

void read_scene(char* filename);

void print_objects();

void normalize(double* v);

void raycasting();

double sphere_intersection(double* Ro, double* Rd, double* C, double r);

double plane_intersection(double* Ro, double* Rd, double* C, double* n);

double sqr(double v);

void write_image_data(char* output_file_name);


// object struct typedef'd as Object intended to hold any of the specified objects in the given scene (.json) file
typedef struct {
  int kind; // 0 = camera, 1 = sphere, 2 = plane
  double color[3]; // potentially remove here and add below
  union {
    struct {
      double width;
      double height;
    } camera;
    struct {
      //double color[3];
      double position[3];
      double radius;
    } sphere;
    struct {
      //double color[3];
	  double position[3];
	  double normal[3];
    } plane;
  };
} Object;

// header_data buffer which is intended to contain all relevant header information of ppm file
typedef struct header_data {
  char* file_format;
  char* file_comment;
  char* file_height;
  char* file_width;
  char* file_maxcolor;
} header_data;

// image_data buffer which is intended to hold a set of RGB pixels represented as unsigned char's
typedef struct image_data {
  unsigned char r, g, b;
} image_data;

int line = 1; // global int line variable to keep track of line in file

// global header_data buffer
header_data *header_buffer;

// global image_data buffer
image_data *image_buffer;

// global set of objects from json file
Object** objects;

double glob_width = 0; // global width, intended to store camera width
double glob_height = 0; // global height, intended to store camera height

int main(int argc, char** argv) {
	
	if(argc != 5) // checks for 5 arguments which includes the argv[0] path argument as well as the 4 required arguments of format [width height input.json output.ppm]
	{
		fprintf(stderr, "Error: Incorrect number of arguments; format should be -> [width height input.json output.ppm]\n");
		return -1;
	}
	
  int width = atoi(argv[1]); // the width of the scene
  int height = atoi(argv[2]); // the height of the scene
  char* input_file = argv[3]; // a .json file to read from
  char* output_file = argv[4]; // a .ppm file to output to
  
  // block of code which checks to make sure that user inputted a .json and .ppm file for both the input and output command line arguments
	char* temp_ptr;
	int input_length = strlen(input_file);
	int output_length = strlen(output_file);
	
	temp_ptr = input_file + (input_length - 5); // sets temp_ptr to be equal to the last 4 characters of the input_name, which should be .ppm
	if(strcmp(temp_ptr, ".json") != 0)
	{
		fprintf(stderr, "Error: Input file must be a .json file\n");
		return -1;
	}
	
	temp_ptr = output_file + (output_length - 4); // sets temp_ptr to be equal to the last 4 characters of the output_name, which should be .ppm
	if(strcmp(temp_ptr, ".ppm") != 0)
	{
		fprintf(stderr, "Error: Output file must be a .ppm file\n");
		return -1;
	}
  // end of .json/.ppm extension error checking	
  
  objects = malloc(sizeof(Object*)*129); // 128 max objects
  
  // block of code allocating memory to global header_buffer before its use
  header_buffer = (struct header_data*)malloc(sizeof(struct header_data)); 
  header_buffer->file_format = (char *)malloc(100);
  header_buffer->file_comment = (char *)malloc(1024);
  header_buffer->file_height = (char *)malloc(100);
  header_buffer->file_width = (char *)malloc(100);
  header_buffer->file_maxcolor = (char *)malloc(100);
  
  strcpy(header_buffer->file_format, "P6");
  sprintf(header_buffer->file_height, "%d", height);
  sprintf(header_buffer->file_width, "%d", width);
  sprintf(header_buffer->file_maxcolor, "%d", 255);
  
  // ALSO DO ERROR CHECKING WITH GIVEN NEXT_NUM/VECTOR FUNC
  
  image_buffer = (image_data *)malloc(sizeof(image_data) * width * height + 1);
  
  read_scene(input_file);
  printf("Finished parsing!\n");
  print_objects();
  printf("About to start raycasting...\n");
  raycasting();
  printf("Done raycasting and determining pixel colors, now writing to file...\n");
  
   // testing code to delete file in order to prevent data overflow
  FILE* test_fp = fopen(output_file, "r");
  //printf("Output file name is: %s\n", output_file);
  if(test_fp != NULL) 
  {
	fclose(test_fp);
	remove(output_file);
  }
  else if(test_fp == NULL) printf("File doesn't exist\n");
  // end of testing code
  
  // ADD ERROR CHECKING FOR RIGHT COLOR VALUE (NOT > MAXCOLOR OR <)
  
  write_image_data(output_file);
  printf("Done writing!\n");
  
  return 0;
}

// function which takes in an origin ray, direction of the ray, position of the sphere object, and radius of the sphere object and determines if there's an intersection at the current point
double sphere_intersection(double* Ro, double* Rd, double* C, double r)
{
	double a = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
	double b = (2 * (Ro[0] * Rd[0] - Rd[0] * C[0] + Ro[1] * Rd[1] - Rd[1] * C[1] + Ro[2] * Rd[2] - Rd[2] * C[2]));
	double c = sqr(Ro[0]) - 2*Ro[0]*C[0] + sqr(C[0]) + sqr(Ro[1]) - 2*Ro[1]*C[1] + sqr(C[1]) + sqr(Ro[2]) - 2*Ro[2]*C[2] + sqr(C[2]) - sqr(r);
		
	double det = sqr(b) - 4 * a * c;
	if(det < 0) return -1;
	
	det = sqrt(det);
	
	double t0 = (-b - det) / (2 * a);
	//printf("t0 value is: %lf\n", t0);
	if(t0 > 0) return t0;
	double t1 = (-b + det) / (2 * a);
	//printf("t1 value is: %lf\n", t1);
	if(t1 > 0) return t1;
	
	//printf("Didn't eval\n");
	
	return -1;
}

// function which takes in an origin ray, direction of the ray, position of the plane object, and normal of the plane object and determines if there's an intersection at the current point
double plane_intersection(double* Ro, double* Rd, double* C, double* N)
{	
	normalize(N);
	double Vd = ((N[0] * Rd[0]) + (N[1] * Rd[1]) + (N[2] * Rd[2]));
	if(Vd == 0) 
	{
		//printf("Parallel ray so no intersection.\n");
		return -1;
	}
	double Vo = -((N[0] * Ro[0]) + (N[1] * Ro[1]) + (N[2] * Ro[2])) + sqrt(sqr(C[0] - Ro[0]) + sqr(C[1] - Ro[1]) + sqr(C[2] - Ro[2]));

	
	double t = Vo/Vd;
		
	if(t > 0) 
	{
		//printf("Found a plane intersection.\n");
		return t;
	}
	//printf("Didn't find a plane intersection.\n");
	
	return -1;
}

// function which handles raycasting for objects read in from json file
void raycasting() 
{
	
		image_data current_pixel; // temp image_data struct which will hold RGB pixels
		image_data* temp_ptr = image_buffer; // + sizeof(image_data) * atoi(header_buffer->file_height) * atoi(header_buffer->file_width); // temp ptr to image_data struct which will be used to navigate through global buffer
		current_pixel.r = 0;
		current_pixel.g = 0; // initializes current pixel RGB values to 255
		current_pixel.b = 0;
		int pixel_counter = 0;
		
		double cx = 0;
		double cy = 0;
		
		int M = atoi(header_buffer->file_height); // change?
		int N = atoi(header_buffer->file_width); // change?
		// ERROR CHECK M & N AGAINS WIDTH X HEIGHT FROM CMD LINE (set them equal to cmd line parameters?
		
		double pixheight = glob_height / M;
		double pixwidth = glob_width / N;
		
		printf("Global height is: %lf and width is: %lf.\n", glob_height, glob_width);
		
		printf("pixheight is: %lf and pixwidth is: %lf\n", pixheight, pixwidth);
		
		double Ro[3] = {0, 0, 0};
		double Rd[3] = {0, 0, 0};
		double ray[3] = {0, 0, 1};
		
		
		for (int y = 0; y < M; y += 1) {
			ray[1] = (cy - (glob_height/2) + pixheight * (y + 0.5));
			for (int x = 0; x < N; x += 1) {
				ray[0] = cx - (glob_width/2) + pixwidth * (x + 0.5);
				// Rd = normalize(P - Ro)
				Rd[0] = ray[0];
				Rd[1] = ray[1];
				Rd[2] = ray[2];
				normalize(Rd);

					double best_t = INFINITY;
					int best_i = 0;
					for (int i=0; objects[i] != 0; i += 1) {
						double t = 0;

						//printf("i value is: %d.\n", i);
						switch(objects[i]->kind) {
						case 0:
							break;
						case 1:
						    //printf("Sphere intersection check.\n");
							t = sphere_intersection(Ro, Rd,
														objects[i]->sphere.position,
														objects[i]->sphere.radius);
														
							
							break;
						case 2:
							//printf("Plane intersect not implemented yet\n");
							t = plane_intersection(Ro, Rd,
														objects[i]->plane.position,
														objects[i]->plane.normal);
							break;
						default:
						// Horrible error -> FLESH out
						// dooooooooooooooooooo
							exit(1);
						}
						if (t > 0 && t < best_t)
						{
							best_t = t;
							best_i = i;
						}
					}
					//printf("Current x is: %d and y is: %d\n", x, y);
					if (best_t > 0 && best_t != INFINITY) {
						//printf("A pixel will be colored here; size of struct is: %d and N is %d, M is %d; pixel spot is: %d\n", sizeof(image_data), N, M, ((sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) - (x * sizeof(image_data))) / 3);
						current_pixel.r = objects[best_i]->color[0] * 255;
						current_pixel.g = objects[best_i]->color[1] * 255;
						current_pixel.b = objects[best_i]->color[2] * 255;
						*(image_buffer + y * N + x) = current_pixel;
						//printf("Got a hit, coloring pixel non-white.\n");
						//printf("Final coloring is: r - %d, g - %d, b - %d\n", current_pixel.r, current_pixel.g, current_pixel.b);
						//double test = (sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) + (x * sizeof(image_data));// + (x * sizeof(image_data));
						//printf("Pixel to be colored is at location: %lf.\n", test);
						//*(temp_ptr + (sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) - (x * sizeof(image_data))) = current_pixel; // + (x * sizeof(image_data))) = current_pixel; // effectively stores current pixel in temporary buffer
						//*temp_ptr = current_pixel;
						//temp_ptr++; // increments temp_ptr to point to next image_data struct in global buffer
						//*(image_buffer+(sizeof(image_data)*M*N)-y*M*sizeof(image_data)+x*sizeof(image_data)) = current_pixel;
						current_pixel.r = 0;
						current_pixel.g = 0; // resets current pixel RGB values to 0 after coloring current pixel
						current_pixel.b = 0;
						printf("#");
					} else {
						//printf("A pixel won't be colored here; size of struct is: %d and N is %d, M is %d; pixel spot is: %d\n", sizeof(image_data), N, M, ((sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) - (x * sizeof(image_data))) / 3);
						
						//printf("Didn't hit, coloring black.");
						current_pixel.r = 0;
						current_pixel.g = 0; // resets current pixel RGB values to 0 since current_pixel is not colored (non-white in this case)
						current_pixel.b = 0;		
						*(image_buffer + y * N + x) = current_pixel;						
						//double test = (sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) + (x * sizeof(image_data));// + (x * sizeof(image_data));
						//printf("Pixel to be not-colored is at location: %lf.\n", test);
						//*(temp_ptr + (sizeof(image_data) * N * M) - (y * sizeof(image_data) * M) - (x * sizeof(image_data))) = current_pixel; // + (x * sizeof(image_data))) = current_pixel; // effectively stores current pixel in temporary buffer
						 //*(image_buffer+(sizeof(image_data)*M*N)-y*M*sizeof(image_data)+x*sizeof(image_data)) = current_pixel;

						//*temp_ptr = current_pixel;
						//temp_ptr++; // increments temp_ptr to point to next image_data struct in global buffer
					    printf(".");
					}
      
			}			
			printf("\n");
			}
  
			// i++;
	//}
			
		return;
}

// squares given double
double sqr(double v) {
  return v*v;
}

// normalizes given vector
void normalize(double* v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

// helper function
void print_objects()
{
	int i = 0;
	while(objects[i] != NULL)
	{
			if(objects[i]->kind == 0)
			{
				printf("#%d object is a camera\n", i);
				printf("Camera width is: %lf\n", objects[i]->camera.width);
				printf("Camera height is: %lf\n", objects[i]->camera.height);
				printf("-------------------------------------------------------\n");
				i++;
			}
			else if(objects[i]->kind == 1)
			{
				printf("#%d object is a sphere\n", i);
				printf("Sphere color is: [%lf, %lf, %lf]\n", objects[i]->color[0], objects[i]->color[1], objects[i]->color[2]);
				printf("Sphere position is: [%lf, %lf, %lf]\n", objects[i]->sphere.position[0], objects[i]->sphere.position[1], objects[i]->sphere.position[2]);
				printf("Sphere radius is: %lf\n", objects[i]->sphere.radius);
				printf("-------------------------------------------------------\n");
				i++;
			}
			else if(objects[i]->kind == 2)
			{
				printf("#%d object is a plane\n", i);
				printf("Plane color is: [%lf, %lf, %lf]\n", objects[i]->color[0], objects[i]->color[1], objects[i]->color[2]);
				printf("Plane position is: [%lf, %lf, %lf]\n", objects[i]->plane.position[0], objects[i]->plane.position[1], objects[i]->plane.position[2]);
				printf("Plane normal is: [%lf, %lf, %lf]\n", objects[i]->plane.normal[0], objects[i]->plane.normal[1], objects[i]->plane.normal[2]);
				printf("-------------------------------------------------------\n");
				i++;
			}
			else
			{
				fprintf(stderr, "Error: Unrecognized object.\n");
				exit(1);
			}

	}
}

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);    
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }  
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);      
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);      
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

// function which reads next number from file, wrapped around error checking if nothing is read in
double next_number(FILE* json) {
  double value;
  if(fscanf(json, "%lf", &value) == 0)
  {
	  fprintf(stderr, "Error: Expected a number on line %d.\n", line);
      exit(1);	
  }
  return value;
}

// function which reads next vector from file
double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  printf("Read in number (vector) is: %lf.\n", v[0]);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  printf("Read in number (vector) is: %lf.\n", v[1]);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  printf("Read in number (vector) is: %lf.\n", v[2]);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}


void read_scene(char* filename) {
  int c;
  FILE* json = fopen(filename, "r");

  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }
  
  skip_ws(json);
  
  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  int i = 0;
  
  // Find the objects

  while (1) {
    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: Empty Scene File.\n");
      fclose(json);
      return;
    }
    if (c == '{') {
      skip_ws(json);
    
      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
		fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
		exit(1);
      }

      skip_ws(json);

      expect_c(json, ':');

      skip_ws(json);

      char* value = next_string(json);

      if (strcmp(value, "camera") == 0) {
		  objects[i] = malloc(sizeof(Object));
		  objects[i]->kind = 0;
		  
		  //i++; // increment i after we no longer need to access the object
		  
		  
      } else if (strcmp(value, "sphere") == 0) {
		  objects[i] = malloc(sizeof(Object));
		  objects[i]->kind = 1;
		  
		 // i++; // increment i after we no longer need to access the object

		  
      } else if (strcmp(value, "plane") == 0) {
		  objects[i] = malloc(sizeof(Object));
		  objects[i]->kind = 2;
		  
		  //i++; // increment i after we no longer need to access the object

		  
      } else {
		fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
		exit(1);
      }

      skip_ws(json);

      while (1) {
	  // , }
	  c = next_c(json);
	  if (c == '}') {
	    // stop parsing this object
	    //printf("Finished parsing object so incrementing iterator. Before increment: %d\n", i);
	    i++;
	    break;
	  } else if (c == ',') {
	  // read another field
	  skip_ws(json);
	  char* key = next_string(json);
	  skip_ws(json);
	  expect_c(json, ':');
	  skip_ws(json);
	  if ((strcmp(key, "width") == 0) ||
	      (strcmp(key, "height") == 0) ||
	      (strcmp(key, "radius") == 0)) {
	    double value = next_number(json);
		if(strcmp(key, "width") == 0)
		{
			printf("Currently assigning width: %f.\n", value);
			objects[i]->camera.width = value;
			glob_width = value; // stores camera width to prevent need to iterate through objects later
		}
		else if(strcmp(key, "height") == 0)
		{
			printf("Currently assigning height %f.\n", value);
			objects[i]->camera.height = value; 
			glob_height = value; // stores camera height to prevent need to iterate through objects later
		}
		else
		{
			printf("Currently assigning radius %f.\n", value);
			objects[i]->sphere.radius = value;
		}
		
	  } else if ((strcmp(key, "color") == 0) ||
		     (strcmp(key, "position") == 0) ||
		     (strcmp(key, "normal") == 0)) { // do additional error checking with color
	    double* value = next_vector(json);
		if(strcmp(key, "color") == 0)
		{
			printf("Currently assigning colors [%lf, %lf, %lf]\n", value[0], value[1], value[2]);
			// ERROR CHECK FOR COLOR VALUE > 0 AND < 256
			objects[i]->color[0] = value[0];
			objects[i]->color[1] = value[1];
			objects[i]->color[2] = value[2];
		}
		else if(strcmp(key, "position") == 0)
		{
			printf("Currently assigning position [%lf, %lf, %lf]\n", value[0], value[1], value[2]);
			if(objects[i]->kind == 1)
			{
				objects[i]->sphere.position[0] = value[0];
				objects[i]->sphere.position[1] = value[1];
				objects[i]->sphere.position[2] = value[2];
			}
			else if(objects[i]->kind == 2)
			{
				objects[i]->plane.position[0] = value[0];
				objects[i]->plane.position[1] = value[1];
				objects[i]->plane.position[2] = value[2];
			}
			else
			{
				fprintf(stderr, "Error: Mismatched object field, on line %d.\n", key, line);
			}
		}
		else
		{
			printf("Currently assigning normal [%lf, %lf, %lf]\n", value[0], value[1], value[2]);
			objects[i]->plane.normal[0] = value[0];
			objects[i]->plane.normal[1] = value[1];
			objects[i]->plane.normal[2] = value[2];
		}
	  } else {
	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
		    key, line);
	    //char* value = next_string(json);
	  }
	  skip_ws(json);
	} else {
		fprintf(stderr, "Error: Unexpected value on line %d. Expected either ',' or '}' to indicate next field or end of object.\n", line);
		exit(1);
	}
      }
      skip_ws(json);
      c = next_c(json);
      if (c == ',') { // Should be followed by another object
		// noop
		skip_ws(json);
      } else if (c == ']') {
			//printf("Reached end of objects so null-terminating and current index is: %d.\n", i);
			objects[i] = NULL; // null-terminate after last object
			fclose(json);
			return;
      } else {
			fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
			exit(1);
      }
    }
    else // didn't find end of file or the beginning of an object
	{
		fprintf(stderr, "Error: Expecting '{' or ']' on line %d.\n", line);
		exit(1);
	}
  }
}

// write_image_data function takes in the output_file_name to know where to open the file
void write_image_data(char* output_file_name)
{
	FILE *fp;
	
	fp = fopen(output_file_name, "a"); // opens file to be appended to (file will be created if one does not exist)
	
	if(fp == NULL) 
	{
		fprintf(stderr, "Error: Output file couldn't be created/modified.\n");
		exit(1); // exits out of program due to error
	}
	
	char* current_line;
	
	// block of code which writes header information into the output file along with whitespaces accordingly
	fprintf(fp, header_buffer->file_format); 
	fprintf(fp, "\n");
	fprintf(fp, header_buffer->file_width);
	fprintf(fp, " ");
	fprintf(fp, header_buffer->file_height);
	fprintf(fp, "\n");
	fprintf(fp, header_buffer->file_maxcolor);
	fprintf(fp, "\n");
	
	// strcmp to check for type of input file format -> will always be P3 in this case
	if(strcmp(header_buffer->file_format, "P3") == 0)
	{		
		int i = 0; // initializes iterator variable
		unsigned char temp[64] = {0}; // creates temp array of unsigned char
		char temp_string[64]; // creates temp_string to hold converted value from file as a string
		image_data* temp_ptr = image_buffer; // temp ptr to image_data struct which will be used to navigate through stored pixels in the global buffer
		
		// while loop which iterates for every pixel in the file using width * height
		while(i < atoi(header_buffer->file_width) * atoi(header_buffer->file_height))
	    {
			sprintf(temp_string, "%d", (*temp_ptr).r); // converts read-in pixel "r" value to a string
			fprintf(fp, temp_string); // writes converted pixel as a string to the file
			memset(temp_string, 0, 64); // resets all values in temp to 0 for reuse
			fprintf(fp, "\n"); // prints a newline to act as "whitespace" between pixel information
			
			sprintf(temp_string, "%d", (*temp_ptr).g); // converts read-in pixel "g" value to a string
			fprintf(fp, temp_string); // writes converted pixel as a string to the file
			memset(temp_string, 0, 64); // resets all values in temp to 0 for reuse
			fprintf(fp, "\n"); // prints a newline to act as "whitespace" between pixel information
			
			sprintf(temp_string, "%d", (*temp_ptr).b); // converts read-in pixel "b" value to a string	
			fprintf(fp, temp_string); // writes converted pixel as a string to the file
			memset(temp_string, 0, 64); // resets all values in temp to 0 for reuse
			fprintf(fp, "\n"); // prints a newline to act as "whitespace" between pixel information
				
			temp_ptr++; // increments temp_ptr to point to next image_data struct in global buffer
			i++;  // increments iterator variable
		}

		fclose(fp);
    }
	else if(strcmp(header_buffer->file_format, "P6") == 0)
	{
		fclose(fp); // closes file after writing header information since P6 requires writing bytes
		fopen(output_file_name, "ab"); // opens file to be appended to in byte mode
		int i = 0; // initializes iterator variable
		image_data* temp_ptr = image_buffer; // temp ptr to image_data struct which will be used to navigate through stored pixels in the global buffer
		
		// while loop which iterates for every pixel in the file using width * height
		while(i != atoi(header_buffer->file_width) * atoi(header_buffer->file_height))
	    {
			fwrite(&(*temp_ptr).r, sizeof(unsigned char), 1, fp); // writes the current pixels "r" value of an "unsigned char" byte to the file
			fwrite(&(*temp_ptr).g, sizeof(unsigned char), 1, fp); // writes the current pixels "g" value of an "unsigned char" byte to the file
			fwrite(&(*temp_ptr).b, sizeof(unsigned char), 1, fp); // writes the current pixels "b" value of an "unsigned char" byte to the file
				
			temp_ptr++; // increments temp_ptr to point to next image_data struct in global buffer
			i++;  // increments iterator variable
		}
		
		fclose(fp);
	}
	
	// extra error checking in case file format given was invalid, but should have been caught earlier
	else
	{
		fprintf(stderr, "Error: File format to write out not recognized\n");
		fclose(fp); // closes file before exiting out
		exit(1); // exits out of program due to error	
	}
}