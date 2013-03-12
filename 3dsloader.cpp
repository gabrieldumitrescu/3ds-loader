/* File 3dsLoader.cpp 
 * Reads a 3ds file and loads the vertices and polygons
 * into a structure 
 */

#include <stdio.h>
#include <string>
#include <queue>

#define MAIN_CHUNK_ID 0x4d4d
#define EDITOR3D_CHUNK_ID 0x3d3d

#define OBJECT_BLOCK_CHUNK_ID 0X4000
#define TRIANGULAR_MESH_CHUNK_ID 0X4100

#define VERTICES_CHUNK_ID 0x4110
#define FACES_CHUNK_ID 0x4120

/*sizeof(ChunkHeader) is 8 because of machine alignment 
 * Must manually code that instead of just reading the
 * header into such a structure
 */
struct ChunkHeader{
  unsigned short chunkId;
  unsigned int chunkLength;
};

/* Stores a 3d vector */
struct Vector3f{
  float x,y,z;
};

struct My3DObject{
  std::string name;
  unsigned short no_vertices,no_faces;
  Vector3f *vertices;
  unsigned short *indices;
};

/* Function readChunkHeader()
 * ---------------------------
 * Parse the first 6 bytes of a chunk's header into the appropriate filed
 * of a ChunkHeader structure
 * File pointer must be at the beginning of a chunk.
 * fread(&chunkHeader,sizeof(ChunkHeader),1,file) introduces a bug 
 * because of machine memory alignment requirements size of the header being 6 
 * and ChunkHeader structure ending up being 8 bytes long. 
 * Each field must thus be read individually.
 */
ChunkHeader readChunkHeader(FILE* file){
  ChunkHeader ch;
  fread(&ch.chunkId,sizeof(unsigned short),1,file);
  fread(&ch.chunkLength,sizeof(unsigned int),1,file);
  return ch;
}


/* Function filelength
 *-----------------------
 * Return the size of a file.
 * ??? maybe it shoud return a double for larger file lengths ????
 */
unsigned int filelength(FILE* file){
  fseek(file,0,SEEK_END);
  unsigned int length=ftell(file);
  fseek(file,0,SEEK_SET);
  return length;
}

/* Function readChunks()
 *----------------------
 * Read a 3ds file and construct 3d objects form the chunks
 * Objects are added to the queue objects for later processing.
 * Information about the file and chunks is printed as the file is read.
 */
int readChunks(const char *filename,std::queue<My3DObject> &objects){
  FILE *dsfile;
  if((dsfile=fopen(filename,"rb"))==NULL) return 0;
  unsigned int length=filelength(dsfile);
  printf("File length is %d\n",length);
  int total_vertices=0;
  My3DObject *object=new My3DObject;
  while(ftell(dsfile)<length){
    ChunkHeader ch=readChunkHeader(dsfile);
    switch (ch.chunkId){
    case MAIN_CHUNK_ID:
      printf("Found main chunk of %d length\n",ch.chunkLength); break;
    case EDITOR3D_CHUNK_ID:
      break;
    case OBJECT_BLOCK_CHUNK_ID:{
      std::string objName;
      char nextChar;
      do{
	fread(&nextChar,sizeof(char),1,dsfile);
	objName+=nextChar;
      }while(nextChar!='\0');
      printf("Reading object  '%s'\n",objName.c_str());
      object->name=objName;
    }
      break;
    case TRIANGULAR_MESH_CHUNK_ID:
      break;
    case VERTICES_CHUNK_ID:{
      unsigned short no_vertices;
      printf("Adding vertices\n");
      fflush(stdout);
      fread(&no_vertices,sizeof(unsigned short),1,dsfile);
      object->vertices=new Vector3f[no_vertices];
      object->no_vertices=no_vertices;
      
      for(unsigned short i=0; i<no_vertices; i++){
	Vector3f vertex;
	fread(&vertex.x,sizeof(float),1,dsfile);
	fread(&vertex.y,sizeof(float),1,dsfile);
	fread(&vertex.z,sizeof(float),1,dsfile);
	object->vertices[i]=vertex;
      }
      total_vertices+=no_vertices;
    }
      break;
    case FACES_CHUNK_ID:{
      unsigned short no_faces;
      printf("Adding indices\n");
      fflush(stdout);
      fread(&no_faces,sizeof(unsigned short),1,dsfile);
      object->indices=new unsigned short[3*no_faces];
      object->no_faces=no_faces;
      for(unsigned short i=0; i<no_faces; i++){
	unsigned short vertex_index;
	fread(&vertex_index,sizeof(unsigned short),1,dsfile);
	object->indices[3*i]=vertex_index;
	fread(&vertex_index,sizeof(unsigned short),1,dsfile);
	object->indices[3*i+1]=vertex_index;
	fread(&vertex_index,sizeof(unsigned short),1,dsfile);
	object->indices[3*i+2]=vertex_index;
	fread(&vertex_index,sizeof(unsigned short),1,dsfile); // discarded, only to advance file pointer
      }
     objects.push(*object);
     delete object;
     object=new My3DObject;
    }
      break;
    default:
      fseek(dsfile,ch.chunkLength-6,SEEK_CUR);
    }
  }
  printf("Total no of vertices: %d\n",total_vertices);
  fclose(dsfile);
  return length;
 }

int main(){
  std::string modelfile="Car.3DS";
  std::queue<My3DObject> *objects=new std::queue<My3DObject>;
  if(readChunks(modelfile.c_str(),*objects)==0){
    printf("Error reading %s file. Make sure it exists and has the right format.",modelfile.c_str());
    return 1;
  }
  while(!objects->empty()){
    My3DObject current=(My3DObject) objects->front();
    printf("Object %s - %d vertices %d faces \n",current.name.c_str(),current.no_vertices,current.no_faces);
    delete[] current.vertices;
    delete[] current.indices;
    objects->pop();
  }
  return 0;
}
