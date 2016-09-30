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


// function prototypes
int next_c(FILE* json);

void expect_c(FILE* json, int d);

void skip_ws(FILE* json);

char* next_string(FILE* json);

double next_number(FILE* json);

double* next_vector(FILE* json);

void read_scene(char* filename);

void print_objects();


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

int line = 1; // global int line variable to keep track of line in file

 Object** objects;

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
  
  objects = malloc(sizeof(Object*)*30);
  
  // ADD ERROR CHECKING FOR CORRECT FILE EXTENSIONS
  
  // ALSO DO ERROR CHECKING WITH GIVEN NEXT_NUM/VECTOR FUNC
  
  read_scene(input_file);
  printf("Finished parsing!\n");
  print_objects();
  
  return 0;
}

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

double next_number(FILE* json) {
  double value;
  //printf("Current file location is: %d\n", ftell(json));
  fscanf(json, "%lf", &value);
  //printf("New file location is: %d\n", ftell(json));
  //printf("Read in number is: %lf\n", value);
  // advance if comma
  // Error check this.. DO THIS
  return value;
}

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

  // allocates memory to objects; may need to rethink this area if more objects are needed
  //Object** objects;
  //objects = malloc(sizeof(Object*)*30); // currently assuming large (and arbitrary) number of objects
  int i = 0;
  
  
  // Find the objects

  while (1) {
    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
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
	  printf("Finished parsing object so incrementing iterator. Before increment: %d\n", i);
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
		}
		else if(strcmp(key, "height") == 0)
		{
			printf("Currently assigning height %f.\n", value);
			objects[i]->camera.height = value;
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
			printf("Currently assigning colors [%d, %d, %d]\n", value[0], value[1], value[2]);
			objects[i]->color[0] = value[0];
			objects[i]->color[1] = value[1];
			objects[i]->color[2] = value[2];
		}
		else if(strcmp(key, "position") == 0)
		{
			printf("Currently assigning position [%d, %d, %d]\n", value[0], value[1], value[2]);
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
			printf("Currently assigning normal [%d, %d, %d]\n", value[0], value[1], value[2]);
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
	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
	  exit(1);
	}
      }
      skip_ws(json);
      c = next_c(json);
      if (c == ',') {
	// noop
	skip_ws(json);
      } else if (c == ']') {
	printf("Reached end of objects so null-terminating and current index is: %d.\n", i);
	objects[i] = NULL; // null-terminate after last object
	fclose(json);
	return;
      } else {
	fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
	exit(1);
      }
    }
  }
}