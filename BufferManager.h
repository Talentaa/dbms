/* This is the BufferManager, DSManager, and Globals */
#pragma once

/* Includes */
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdio.h>
using namespace std;
/* Defines */
#define MAXDBS 10
#define MAXATTRS                                                               \
  100 ///////////////////KNUTE////////////////////////////////////
#define MAXTYPES 15
#define MAXRELS 1000
#define MAXINDEXES 1000
#define MAXONS 1000
#define MAXPAGES 100000
#define DEFBUFSIZE 1024
#define FRAMESIZE 4096
/* Structs */
/* This struct defines what a buffer frame looks like */
struct bFrame {
  char field[FRAMESIZE];
};

/* This is the LRU Element Struct */
struct LRUEle {
  LRUEle();
  int fid;
  double b2dist;
  LRUEle *less_recent;
  LRUEle *more_recent;
};

/* This is the NewPage return struct */
struct NewPage {
  int page_id;
  int frame_id;
};

/* This is the Buffer Control Block Struct */
struct BCB {
  BCB();
  int page_id;
  int frame_id;
  int latch;
  int count;
  int ftime;
  int stime;
  int dirty;
  BCB *next;
};
/* This struct contains a Database */
struct DB {
  char name[100];
  char owner[100];
  char group[100];
  char date_modified[100];
  char create_date[100];
};
/* This struct keeps track of a relation table */
struct Rel {
  char DBName[100];
  char name[100];
  char type[100];
  int size;
  int first_page;
  int last_page;
};
/* This relation contains an Attribute relation */
struct Attr {
  char DBName[100];
  char Relname[100];
  char name[100];
  int length;
  int precision;
  int position;
  int IsUnique; /*KNUTE???????????????????????????????????????????????????????????????????????????????????????????*/
  int IsPrimary; /* 0 = not primary, 1 = primary */
  char TypeName[100];
};

/* This struct contains a type relation */
struct Type {
  char name[100];
  int size;
};
/* This struct contains an index relation */
struct Index {
  char DBName[100];
  char name[100];
  int unique;
  int header_page;
};
// This struct contains an On relation
struct On {
  char DBName1[100];
  char IndexName[100];
  char DBName2[100];
  char Relname[100];
  char name[100];
};
/* This is the struct that defines how the metadata will
   be stored */
struct Metadata {
  /* Constructors */
  Metadata();
  /* Interface functions */
  int InitMetadata();
  int WriteMetadata();
  int ClearDB(int index);
  int ClearRel(int index);
  int ClearAttr(int index);
  int ClearType(int index);
  int ClearIndex(int index);
  int ClearOn(int index);
  void printDB(int index);
  void printAllDB();
  void printRel(int index);
  void printAllRel();
  void printAttr(int index);
  void printAllAttr();
  void printType(int index);
  void printAllTypes();
  void printIndex(int index);
  void printAllIndexes();
  void printOn(int index);
  void printAllOns();
  void printAll();
  void describeDatabase(const char DBName[100]);
  void describeTable(const char dname[100], const char tname[100]);

  /* File & Access */
  int Create_db(const char *DB, char *Owner, char *Mdate, char *Cdate);
  int Create_db(const char *DB, const char *Owner, const char *Mdate,
                const char *Cdate);
  int Drop_db(const char *DB);
  int Create_table(const char *DB, const char *Table);
  int Drop_table(const char *DB, const char *Table);
  int Add_attribute(const char *DB, const char *Table, const char *AttName,
                    const char *AttType, int leng, int prec, int position,
                    int IsPrimary);
  int Drop_attribute(const char *DB, const char *Table, const char *AttName);

  /* Member variables */
  DB dbases[MAXDBS];
  Rel relations[MAXRELS];
  Attr attrs[MAXATTRS];
  Type types[MAXTYPES];
  Index indexes[MAXINDEXES];
  On ons[MAXONS];
};
/* Classes */

/* This Is the Disk Manager class
   All functions for this class have to do with files */
class DSMgr {
public:
  /* Constructor */
  DSMgr();
  /* Interface functions */
  int OpenFile(string filename);
  int CloseFile();
  bFrame ReadPage(int page_id);
  int WritePage(int page_id, bFrame frm);
  int Seek(int offset, int pos);
  FILE *getFile();
  void incNumPages();
  int getNumPages();
  void setUse(int index, int use_bit);
  int getUse(int index);

private:
  FILE *currFile;
  int numPages;
  /* This array keeps track of the free pages in the
     dataspace */
  int pages[MAXPAGES];
};

/* This is the Buffer Manager Class
   All functions for this class have to do with the buffer */
class BMgr {
public:
  /* Constructor */
  BMgr();
  /* Interface functions */
  int FixPage(int page_id, int prot);
  NewPage FixNewPage();
  int UnfixPage(int page_id);
  void Lock(int page_id, int prot);
  int NumFreeFrames();
  void printValue(int frame_id, int offset, int len);
  /* Extra Functions */
  int SelectVictim();
  int PageToFrame(int page_id);
  int Hash(int page_id);
  void RemoveBCB(BCB *ptr, int page_id);
  void RemoveLRUEle(int frid);
  void calcLRUList(BCB *ptr, int frid);
  void printFrame(int index);
  void setDirty(int frame_id);
  void writeDirtys();

private:
  int ftop[DEFBUFSIZE];
  /* Hash Table */
  BCB *ptof[DEFBUFSIZE];
};

/*********************************************/
/*                                           */
/*         GLOBALS                           */
/*                                           */
/*********************************************/
/* Global Variables */
DSMgr dsMan;
BMgr bufMan;
Metadata mdata;
/* BUFFER */
bFrame buf[DEFBUFSIZE];

/* LRU Pointers */
LRUEle *lru;
LRUEle *mru;

/* Function Definitions */
/* Structs */
/* LRU Structs */
/* Constructor */
LRUEle::LRUEle() {
  fid = -1;
  b2dist = 0.0;
  less_recent = NULL;
  more_recent = NULL;
}

// BCB Struct
// Constructors
BCB::BCB() {
  next = NULL;
  latch = -1;
  count = -1;
  page_id = -1;
  frame_id = -1;
}
/* Metadata Structs */
/* Constructors */
/* This is the default Constructor.  All values are initialized to \0 and -1
   These values will be updated by the reading in of the metadata file */
Metadata::Metadata() {
  int i = 0;
  /* Clear out all the Metadata */
  for (i = 0; i < MAXDBS; i++) {
    ClearDB(i);
  }
  for (i = 0; i < MAXRELS; i++) {
    ClearRel(i);
  }
  for (i = 0; i < MAXATTRS; i++) {
    ClearAttr(i);
  }
  for (i = 0; i < MAXTYPES; i++) {
    ClearType(i);
  }
  for (i = 0; i < MAXINDEXES; i++) {
    ClearIndex(i);
  }
  for (i = 0; i < MAXONS; i++) {
    ClearOn(i);
  }
}

/* Function Definitions */
/* This function loads the metadata into the structs
   It returns an error code of 0 if successful, 1 if not */
int Metadata::InitMetadata() {
  int i = 0;
  lru = NULL;
  mru = NULL;
  /* Open the file */
  FILE *fd = NULL;
  fd = fopen("./config/metadata.rdb", "r");
  /* If the fd is null, open failed for some reason */
  if (fd != NULL) {
    /* Read in the metadata from the file */
    for (i = 0; i < MAXDBS; i++) {
      fread(&dbases[i], sizeof(DB), 1, fd);
    }
    for (i = 0; i < MAXRELS; i++) {
      fread(&relations[i], sizeof(Rel), 1, fd);
    }
    for (i = 0; i < MAXATTRS; i++) {
      fread(&attrs[i], sizeof(Attr), 1, fd);
    }
    for (i = 0; i < MAXINDEXES; i++) {
      fread(&indexes[i], sizeof(Index), 1, fd);
    }
    for (i = 0; i < MAXONS; i++) {
      fread(&ons[i], sizeof(On), 1, fd);
    }
    fclose(fd);
    /* The Types are hardcoded
       Supported Types at the moment are:
       int, char, float */
    strcpy(types[0].name, "int");
    types[0].size = sizeof(int);
    strcpy(types[1].name, "char");
    types[1].size = sizeof(char);
    strcpy(types[2].name, "float");
    types[2].size = sizeof(float);
  } else {
    /*cout << "FAILED TO FIND THE METADATA FILE\n"; */
    /* The Types are hardcoded
       Supported Types at the moment are:
       int, char, float */
    strcpy(types[0].name, "int");
    types[0].size = sizeof(int);
    strcpy(types[1].name, "char");
    types[1].size = sizeof(char);
    strcpy(types[2].name, "float");
    types[2].size = sizeof(float);
    return 1;
  }
  return 0;
}
// This function writes the metadata to the HDD
// It returns an error code of 0 if successful, 1 if not
int Metadata::WriteMetadata() {
  int i = 0;
  FILE *fd = NULL;
  fd = fopen("./config/metadata.rdb", "w");
  if (fd != NULL) {
    for (i = 0; i < MAXDBS; i++) {
      fwrite(&dbases[i], sizeof(DB), 1, fd);
    }
    for (i = 0; i < MAXRELS; i++) {
      fwrite(&relations[i], sizeof(Rel), 1, fd);
    }
    for (i = 0; i < MAXATTRS; i++) {
      fwrite(&attrs[i], sizeof(Attr), 1, fd);
    }
    for (i = 0; i < MAXINDEXES; i++) {
      fwrite(&indexes[i], sizeof(Index), 1, fd);
    }
    for (i = 0; i < MAXONS; i++) {
      fwrite(&ons[i], sizeof(On), 1, fd);
    }
    fclose(fd);
    /* The types are hardcoded */
  } else {
    return 1;
  }
  return 0;
}

/* This function clears a DB struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearDB(int index) {
  if (index >= MAXDBS) {
    return 1;
  }
  strcpy(dbases[index].group, "\0");
  strcpy(dbases[index].name, "\0");
  strcpy(dbases[index].owner, "\0");
  strcpy(dbases[index].date_modified, "\0");
  strcpy(dbases[index].create_date, "\0");
  return 0;
}

/* This function clears a Rel struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearRel(int index) {
  if (index >= MAXRELS) {
    return 1;
  }
  strcpy(relations[index].DBName, "\0");
  strcpy(relations[index].name, "\0");
  strcpy(relations[index].type, "\0");
  relations[index].first_page = -1;
  relations[index].last_page = -1;
  relations[index].size = -1;
  return 0;
}

/* This function clears a Attr struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearAttr(int index) {
  if (index >= MAXATTRS) {
    return 1;
  }
  strcpy(attrs[index].DBName, "\0");
  strcpy(attrs[index].name, "\0");
  strcpy(attrs[index].Relname, "\0");
  strcpy(attrs[index].TypeName, "\0");
  attrs[index].IsPrimary = -1;
  attrs[index].length = -1;
  attrs[index].position = -1;
  attrs[index].precision = -1;
  return 0;
}

/* This function clears a Type struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearType(int index) {
  if (index >= MAXTYPES) {
    return 1;
  }
  strcpy(types[index].name, "\0");
  types[index].size = -1;
  return 0;
}

/* This function clears a Index struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearIndex(int index) {
  if (index >= MAXINDEXES) {
    return 1;
  }
  strcpy(indexes[index].DBName, "\0");
  strcpy(indexes[index].name, "\0");
  indexes[index].header_page = -1;
  indexes[index].unique = -1;
  return 0;
}

/* This function clears a On struct
   returns 0 if successful, 1 if index out of bounds */
int Metadata::ClearOn(int index) {
  if (index >= MAXONS) {
    return 1;
  }
  strcpy(ons[index].DBName1, "\0");
  strcpy(ons[index].DBName2, "\0");
  strcpy(ons[index].IndexName, "\0");
  strcpy(ons[index].name, "\0");
  strcpy(ons[index].Relname, "\0");
  return 0;
}

/* This function prints the DB referenced by index */
void Metadata::printDB(int index) {
  cout << "Name: " << dbases[index].name << endl;
  cout << "Owner: " << dbases[index].owner << endl;
  cout << "Creation Date: " << dbases[index].create_date << endl;
  cout << "Date Modified: " << dbases[index].date_modified << endl << endl;
}

/* This function calls printDB on all DB structs to output each one */
void Metadata::printAllDB() {
  int i = 0;
  for (i = 0; i < MAXDBS; i++) {
    printDB(i);
  }
}

/* This function prints the Rel referenced by index */
void Metadata::printRel(int index) {
  cout << "Name: " << relations[index].name << endl;
  cout << "DBName: " << relations[index].DBName << endl;
  cout << "Type: " << relations[index].type << endl;
  cout << "Size: " << relations[index].size << endl;
  cout << "First Page: " << relations[index].first_page << endl;
  cout << "Last Page: " << relations[index].last_page << endl << endl;
}

/* This function calls printRel on all relation structs to output them */
void Metadata::printAllRel() {
  int i = 0;
  for (i = 0; i < MAXRELS; i++) {
    printRel(i);
  }
}

/* This function calls printAttr on all the instances of Attr
   in the attrs array */
void Metadata::printAllAttr() {
  int i = 0;
  for (i = 0; i < MAXATTRS; i++) {
    printAttr(i);
  }
}

/* This function prints the Index struct referenced by index */
void Metadata::printIndex(int index) {
  cout << "Name: " << indexes[index].name << endl;
  cout << "DBName: " << indexes[index].DBName << endl;
  cout << "Unique: " << indexes[index].unique << endl;
  cout << "Header Page: " << indexes[index].header_page << endl;
}

/* This function calls printIndex on all the instances
   of Index in the indexes array */
void Metadata::printAllIndexes() {
  int i = 0;
  for (i = 0; i < MAXINDEXES; i++) {
    printIndex(i);
  }
}

/* This function prints the type struct referenced by index */
void Metadata::printType(int index) {
  cout << "Name: " << types[index].name << endl;
  cout << "Size: " << types[index].size << endl;
}

/* This function calls printType to print all of the
   Type instances in the types array */
void Metadata::printAllTypes() {
  int i = 0;
  for (i = 0; i < MAXTYPES; i++) {
    printType(i);
  }
}

/* This function prints the on struct referenced by index */
void Metadata::printOn(int index) {
  cout << "Name: " << ons[index].name << endl;
  cout << "DBName1: " << ons[index].DBName1 << endl;
  cout << "DBName2: " << ons[index].DBName2 << endl;
  cout << "IndexName: " << ons[index].IndexName << endl;
  cout << "Relname: " << ons[index].Relname << endl;
}

/* This function calls printOn on all the instances of On in the
   ons array */
void Metadata::printAllOns() {
  int i = 0;
  for (i = 0; i < MAXONS; i++) {
    printOn(i);
  }
}

/* This function prints everything in the metadata */
void Metadata::printAll() {
  cout << "DBS:\n";
  printAllDB();
  cout << "Rel\n";
  printAllRel();
  cout << "Attrs:\n";
  printAllAttr();
  cout << "Types:\n";
  printAllTypes();
  cout << "Indexes:\n";
  printAllIndexes();
  cout << "Ons:\n";
  printAllOns();
}

/* Disk Space Manager */
/* Constructor */
DSMgr::DSMgr() {
  currFile = NULL;
  int i = 0;
  for (i = 0; i < MAXPAGES; i++) {
    pages[i] = -1;
  }
}

/* Member Functions */
/* This function takes a filename and attempts to open it
   it returns 0 on success and 1 on failure */
int DSMgr::OpenFile(string filename) {
  /* Open the file */
  currFile = fopen(filename.c_str(), "r+");
  /* if the file is not opened */
  if (currFile == NULL) {
    return 1;
  }
  /* Move the file pointer to the end of the file */
  fseek(currFile, 0, SEEK_END);
  /* Calculate the number of pages that the file has */
  numPages = (ftell(currFile) / FRAMESIZE);
  /* Move the file pointer to the start of the file */
  fseek(currFile, 0, SEEK_SET);
  return 0;
}

/* This function tries to close the current file */
int DSMgr::CloseFile() {
  /* Close the file */
  fclose(currFile);
  currFile = NULL; /* Assign the file pointer to NULL */
  return 0;
}

/* This function attempts to read in a page from the file */
bFrame DSMgr::ReadPage(int page_id) {
  int iread = 0;
  bFrame temp;
  /* Move the pointer to the page in the file */
  fseek(currFile, page_id * FRAMESIZE, SEEK_SET);
  /* read in the page */
  iread = (int)fread(&temp, sizeof(temp), 1, currFile);
  return temp;
}

/* This function attempts to write a page to the file
   It returns the number of bytes that were written */
int DSMgr::WritePage(int page_id, bFrame frm) {
  int bytesw = 0;
  /* Move the pointer to the page in the file */
  fseek(currFile, page_id * FRAMESIZE, SEEK_SET);
  /* Overwrite the page */
  bytesw = (int)fwrite(&frm, sizeof(frm), 1, currFile);
  return bytesw;
}

/* This function returns the current file * */
FILE *DSMgr::getFile() { return currFile; }

/* This function moves the pointer in the current file */
int DSMgr::Seek(int offset, int pos) {
  fseek(currFile, offset, pos);
  return 0;
}

/* This Function increments the page counter */
void DSMgr::incNumPages() { numPages++; }

/* This function returns the page counter */
int DSMgr::getNumPages() { return numPages; }

/* This function sets the pages array at index to use_bit
   use_bit: 0 - unused, 1 - in use */
void DSMgr::setUse(int index, int use_bit) { pages[index] = use_bit; }

/* This function gets the use_bit for the page index
   use_bit: 0 - unused, 1 - in use */
int DSMgr::getUse(int index) { return pages[index]; }
/* The Buffer Manager */
/* The Constructor */
BMgr::BMgr() {
  /* Emptying the two tables */
  int i = 0;
  for (i = 0; i < DEFBUFSIZE; i++) {
    ptof[i] = NULL;
    ftop[i] = -1;
  }
  /* Opening the DataSpace file */
  dsMan.OpenFile("dataspace.rdb");
}
/* Member Functions */
/* This function searches the buffer for the page
   If the page is found, the frame_id is returned, else
   a victim is selected and that frame replaced.
   This function returns the frame_id on success or -1 on failure */
int BMgr::FixPage(int page_id, int prot) {
  /* cout << "BM FIX PAGE\n"; */
  /* Get the buffer control block bucket for the page_id */
  int bcb = Hash(page_id);
  int fid = -1;
  BCB *ptr = ptof[bcb];
  /* Look for the BCB * that corresponds to the page_id
     This means that the page is already in the buffer */
  while (ptr != NULL) {
    if (ptr->page_id == page_id) {
      break;
    }
    ptr = ptr->next;
  }
  /* If the ptr is not null, update the LRU-2 list */
  if (ptr != NULL) {
    if (ptr->stime != -1) {
      ptr->ftime = ptr->stime;
    }
    ptr->stime = static_cast<unsigned>(time(0));
    /* ptr->count = ptr->count+1;
       All counts are zero for this version */
    /* Calculate the new LRU list */
    calcLRUList(ptr, ptr->frame_id);
    return ptr->frame_id;
  }
  /* If the ptr is null, the page is not found in the buffer */
  else {
    ptr = ptof[bcb];
    /* Since the page is not in the buffer, a victim must
       be selected. */
    fid = SelectVictim();
    // cout << "VICTIM: " << fid << endl;
    buf[fid] = dsMan.ReadPage(page_id); /* Read in the page */
    ftop[fid] = page_id;                /* Update the frame hash array */
    if (ptr != NULL) {
      /* This loop pulls the ptr to the end of
         the bucket chain */
      while (ptr->next != NULL) {
        ptr = ptr->next;
      }
      /* Create a new buffer control block */
      ptr->next = new BCB();
      ptr = ptr->next;
    }
    /* If the ptr is null, this frame has never been used */
    else {
      ptr = new BCB();
      ptof[bcb] = ptr; // HUGE FIX For Update problem !!!!!!!!!!! TAKING 3 DAYS
                       // IN A RAW!!!!!! 04/16/2006
    }
    /* Init the block values */
    ptr->next = NULL;
    ptr->page_id = page_id;
    ptr->frame_id = fid;
    ptr->latch = 0;
    ptr->count = 0; /* all counts for this version are zero */
    ptr->ftime = static_cast<unsigned>(time(0));
    ptr->stime = -1;
    /* calculate the backward-2 distance and add the frame
       to the LRU list */
    calcLRUList(ptr, fid);
  }
  return fid;
}
/* This function creates a new page and inserts it into the
   buffer and updates the two hash arrays */
/* The NewPage frame_id and page_id members are set to -1 if
   the file size max has been reached */
NewPage BMgr::FixNewPage() {
  /* cout << "BM FIX NEW PAGE\n"; */
  NewPage temp;
  int newPage = -1;
  int i = 0;
  temp.frame_id = 0;
  temp.page_id = 0;
  /* Look for a page that has already been used but the data
     in it has been deleted.  This page can now be used for
     the new page */
  for (i = 0; i < dsMan.getNumPages(); i++) {
    if (dsMan.getUse(i) == 0) {
      newPage = i;
    }
  }
  /* Make sure the file does not climb to be bigger than the
     max allowed size */
  if (dsMan.getNumPages() == MAXPAGES) {
    cout << "MAX DATASPACE REACHED\n";
    temp.frame_id = -1;
    temp.page_id = -1;
    return temp;
  }
  if (newPage == -1) {
    dsMan.incNumPages(); /* Increase the number of pages */
    newPage = dsMan.getNumPages() - 1;
    /* Look for the end of the file and then increase it */
    dsMan.Seek(FRAMESIZE, SEEK_END);
    dsMan.Seek(newPage * FRAMESIZE, SEEK_SET);
  }
  /* Set the DSMgr array at index to in use */
  dsMan.setUse(newPage, 1);
  /* Select the victim frame */
  int fid = SelectVictim();
  /* Load the page into the buffer */
  buf[fid] = dsMan.ReadPage(newPage);
  ftop[fid] = newPage;
  /* Look for the buffer control block that will
     correspond to the page_id */
  int bcb = Hash(newPage);
  BCB *ptr = ptof[bcb];
  temp.page_id = newPage;
  temp.frame_id = fid;
  /* is the bucket empty ? */
  if (ptr == NULL) {
    /* Init the BCB values */
    ptof[bcb] = new BCB();
    ptr = ptof[bcb];
    ptr->next = NULL;
    ptr->page_id = newPage;
    ptr->frame_id = fid;
    ptr->count = 0; /* All counts in this version are zero */
    ptr->latch = 0;
  } else {
    /* Look for the end of the bucket chain */
    while (ptr->next != NULL) {
      ptr = ptr->next;
    }
    /* Init the BCB values */
    ptr->next = new BCB();
    ptr = ptr->next;
    ptr->next = NULL;
    ptr->page_id = newPage;
    ptr->frame_id = fid;
    ptr->count = 0; /* All counts in this version are zero */
    ptr->latch = 0;
  }
  /* Calculate the new LRU-2 list */
  calcLRUList(ptr, fid);
  return temp;
}

/* This function decrements the fix_count and releases
   releases the latch if the fix_count drops to zero */
/* This function is not used in the current version */
int BMgr::UnfixPage(int page_id) {
  /*cout << "BM UNFIX PAGE\n"; */
  /* Look for the BCB that corresponds to the page_id */
  int bcb = Hash(page_id);
  BCB *ptr = ptof[bcb];
  if (ptr == NULL) {
    /* If the ptr is null, something is wrong
       The page_id might be wrong */
    return 1;
  }
  /* Look for the appropriate BCB for the page_id */
  while (ptr->page_id != page_id) {
    ptr = ptr->next;
  }
  if (ptr == NULL) {
    /* If the ptr is null, something is wrong
       The page_id might be wrong */
    return 1;
  }
  /* decrement the fix_count */
  ptr->count = ptr->count - 1;
  /* If the fix_count is zero, release the latch */
  if (ptr->count == 0) {
    ptr->latch = 0;
  }
  return 0;
}

/* This function locks the frame in place
   prot 0 - unlock, 1 - lock */
/* not used in this version */
void BMgr::Lock(int page_id, int prot) {
  /*cout << "BM LOCK\n"; */
  /* Look for the bucket that corresponds to the page_id */
  int bcb = Hash(page_id);
  BCB *ptr = NULL;
  ptr = ptof[bcb];
  /* Look for the BCB for the page_id */
  while (ptr != NULL && ptr->page_id != page_id) {
    ptr = ptr->next;
  }
  /* If the ptr is null, do nothing */
  /* Else, assign the lock to the ptr */
  if (ptr != NULL) {
    ptr->latch = prot;
  }
}

/* This function selects a frame to replace
   If the selected frame needs to be written, it will
   also do this */
int BMgr::SelectVictim() {
  int victimFrame = 0;
  int pid = 0;
  int bcb = 0;
  bool found = false;
  LRUEle *temp = lru;
  /* If the LRU is empty, start with the first buffer frame */
  if (temp == NULL) {
    /*cout << "Empty LRU\n";*/
    return 0;
  }
  BCB *ptr = NULL;
  /* If any frame is empty, use it before selecting another */
  int i = 0;
  for (i = 0; i < DEFBUFSIZE; i++) {
    if (ftop[i] == -1) {
      return i;
    }
  }
  /* Look at the LRU list for replacement */
  victimFrame = temp->fid;
  pid = ftop[victimFrame];
  bcb = Hash(pid);
  ptr = ptof[bcb];
  /* Look for a frame to use */
  while (found != true) {
    /* if the ptr is null, break the loop */
    if (ptr == NULL) {
      break;
    }
    /* Look for the BCB that is the victim frame */
    while (ptr->frame_id != victimFrame) {
      ptr = ptr->next;
    }
    /* Make sure that the frame can be used */
    if (ptr->count == 0 && ptr->latch == 0) {
      found = true;
    }
    /* If the frame cannot be used, move up the LRU list,
       looking for another frame */
    else {
      temp = temp->more_recent;
      /* To prevent access errors */
      if (temp == NULL) {
        break;
      }
      victimFrame = temp->fid;
      pid = ftop[victimFrame];
      bcb = Hash(pid);
      ptr = ptof[bcb];
    }
  }
  /* Use the victimFrame that was found above */
  pid = ftop[victimFrame];
  bcb = Hash(pid);
  ptr = ptof[bcb];
  if (ptr != NULL) {
    if (ptr->next != NULL) {
      /* Look for the BCB that corresponds to the page_id */
      while (ptr != NULL && ptr->page_id != pid) {
        ptr = ptr->next;
      }
      /* If the page has been modified, write it out */
      if (ptr != NULL && ptr->dirty == 1) {
        dsMan.WritePage(pid, buf[victimFrame]);
      }
      /* remove the entries for this page from the LRU list
         and the ptof array */
      if (ptr != NULL) {
        RemoveLRUEle(victimFrame);
        RemoveBCB(ptr, pid);
      }
    }
  }
  /*if(ptr == NULL)
    {
      victimFrame++;
    }*/
  return victimFrame;
}

/* This function removes the Buffer Control Block for the page_id
   from the ptof array.  This is only called if the SelectVictim()
   function needs to replace a frame */
void BMgr::RemoveBCB(BCB *ptr, int page_id) {
  /* Obtain the BCB that corresponds to the page_id */
  BCB *sptr = ptof[Hash(page_id)];
  BCB *tmp = NULL;
  /* if the two pointers are the same, remove it from the list */
  if (sptr == ptr) {
    /* If the chain is longer than 1, need to keep the other buckets */
    if (sptr->next != NULL) {
      sptr = sptr->next;
      delete ptr;
      ptof[Hash(page_id)] = sptr;
      ptr = NULL;
      sptr = NULL;
    }
    /* If not, then don't worry about it */
    else {
      delete ptr;
      ptof[Hash(page_id)] = NULL;
      ptr = NULL;
      sptr = NULL;
    }
  }
  /* Need to look for the correct BCB */
  else {
    while (sptr->next != ptr) {
      sptr = sptr->next;
    }
    sptr->next = ptr->next;
    delete ptr;
    ptr = NULL;
  }
}

/* This function translates a page_id into it's corresponding
   frame_id.  The page must be in the buffer */
int BMgr::PageToFrame(int page_id) {
  int val = Hash(page_id);
  int ret = 0;
  BCB *ptr = NULL;
  ptr = ptof[ret];
  /* Look for the correct BCB */
  while (ptr != NULL && ptr->page_id != page_id) {
    ptr = ptr->next;
  }
  return ptr->frame_id;
}

/* This function removes the LRUEle from the list */
void BMgr::RemoveLRUEle(int frid) {
  LRUEle *ptr = mru;
  /* If the list is empty, there is nothing to do */
  if (ptr == NULL) {
    return;
  }
  /* Look for the correct LRU element in the list
     by moving down through the list moving from the
     most recent to the least recent */
  while (ptr != NULL && ptr->fid != frid) {
    ptr = ptr->less_recent;
  }
  /* If the ptr is null, the element was not found, so do
     nothing */
  if (ptr == NULL) {
    return;
  }
  /* Otherwise, remove the element in the traditional linked
     list fashion.  Make sure to assign the pointers correctly */
  LRUEle *temp;
  ptr->less_recent->less_recent->more_recent = ptr;
  temp = ptr->less_recent->less_recent;
  delete ptr->less_recent;
  /* Bridge the gap */
  ptr->less_recent = temp;
}

/* This function recalculates the backward 2 distance for the BCB block ptr */
void BMgr::calcLRUList(BCB *ptr, int frid) {
  /* If the list is empty, add an element */
  if (mru == NULL) {
    mru = new LRUEle();
    mru->fid = frid;
    mru->less_recent = NULL;
    mru->more_recent = NULL;
    if (ptr->stime != -1) {
      mru->b2dist = (ptr->stime - ptr->ftime);
      // cout << "MRU B2DIST: " << mru->b2dist << endl;
    } else {
      mru->b2dist = -1;
    }
    lru = mru;
  }
  /* Otherwise, look for the correct place to insert the new element */
  else {
    /* Calculate the Backward-2 distance */
    double b = 0.0;
    if (ptr->stime != -1) {
      b = ptr->stime - ptr->ftime;
    }
    /* If the page has only been used once, the b-2 distance is assigned
       to -1 */
    else {
      b = -1;
    }
    LRUEle *pt = mru;
    /* if the element has only been accessed once */
    if (b == -1) {
      /* Make sure that the element is added before any other
         elements that also have a b-2 distance of -1 */
      while (pt->less_recent != NULL && pt->less_recent->b2dist != -1) {
        pt = pt->less_recent;
      }
      /* If the ptr is null, no other -1's have been found */
      if (pt->less_recent == NULL) {
        /* Create a new LRUEle element and init it */
        pt->less_recent = new LRUEle();
        pt->less_recent->more_recent = pt;
        pt = pt->less_recent;
        pt->fid = frid;
        pt->less_recent = NULL;
        pt->b2dist = b;
        /* Update the new LRU */
        lru = pt;
      }
      /* Insert the element before the other -1's */
      else {
        /* Init the temp */
        LRUEle *temp = new LRUEle();
        temp->fid = frid;
        temp->b2dist = b;
        temp->less_recent = pt->less_recent;
        temp->more_recent = pt;
        /* Update the affected pointers */
        pt->less_recent->more_recent = temp;
        pt->less_recent = temp;
      }
    }
    /* Else, the element has been accessed more than once */
    else {
      /* Look for other elements that have a distance of greater
         than the new element */
      while (pt->less_recent != NULL && b > pt->less_recent->b2dist) {
        pt = pt->less_recent;
      }
      /* If the ptr is null, this element has the greatest b2dist */
      if (pt->less_recent == NULL) {
        /* Create a new element and init it */
        pt->less_recent = new LRUEle();
        pt->less_recent->more_recent = pt;
        pt = pt->less_recent;
        pt->b2dist = b;
        pt->fid = frid;
        pt->less_recent = NULL;
        /* Update the LRU */
        lru = pt;
      }
      /* Need to Insert the new element in the middle */
      else {
        /* Create a temp element and init it */
        LRUEle *temp = new LRUEle();
        temp->b2dist = b;
        temp->fid = frid;
        temp->less_recent = pt->less_recent;
        temp->more_recent = pt;
        /* Update the affected pointers */
        pt->less_recent->more_recent = temp;
        pt->less_recent = temp;
      }
    }
  }
}

/* This function takes the page_id and hashes it
   to an index for the ptof hash table */
int BMgr::Hash(int page_id) {
  /* cout << "BM Hashing\n"; */
  int bucket = 0;
  /* A very simple hash, can use more complex ones */
  bucket = page_id % DEFBUFSIZE;
  return bucket;
}

/* This function is used to calculate the number of possible
   usable frames for sorting and other methods */
int BMgr::NumFreeFrames() {
  /* cout << "BM NUMFREEFRAMES\n"; */
  int i = 0;
  int j = 0;
  int num = 0;
  int nFree = 0;
  BCB *ptr = NULL;
  /* Loop through the buffer */
  for (i = 0; i < DEFBUFSIZE; i++) {
    /* Obtain the page_id */
    j = ftop[i];
    /* This frame has not been used yet */
    if (j == -1) {
      nFree++;
    } else {
      /* Obtain the BCB */
      num = Hash(j);
      ptr = ptof[num];
      /* Look for the BCB for the page_id */
      while (ptr != NULL && ptr->frame_id != j) {
        ptr = ptr->next;
      }
      if (ptr != NULL && ptr->frame_id == j) {
        /* Make sure that the frame can be used */
        if (ptr->count == 0 && ptr->latch == 0) {
          nFree++;
        }
      }
    }
  }
  return nFree;
}

/* This function fully prints the frame referenced
   by the index */
void BMgr::printFrame(int index) { cout << buf[index].field << endl; }

/* This function sets the dirty bit to 1
   it needs to be called anytime a frame
   is going to be physically changed.  the
   dirty bit tells whether or not the frame needs
   to be written before it can be selected as
   a victim */
void BMgr::setDirty(int frame_id) {
  /* Obtain the page_id and the BCB start ptr */
  int pid = ftop[frame_id];
  int bcb = Hash(pid);
  BCB *ptr = ptof[bcb];
  /* Look for the correct BCB */
  while (ptr != NULL && ptr->page_id != pid) {
    ptr = ptr->next;
  }
  /* If the ptr is not null, assign the dirty bit */
  if (ptr != NULL) {
    ptr->dirty = 1;
  }
  /* If the ptr is null, something is wrong, perhaps
     the frame_id */
}

/* This function needs to be called in order to write
   all frames that have dirty bits set to 1.
   This function will be called when the system is shut
   down or any time that you may have down time */
void BMgr::writeDirtys() {
  int i = 0;
  BCB *ptr = NULL;
  /* Loop through each frame */
  for (i = 0; i < DEFBUFSIZE; i++) {
    /* get the BCB ptr */
    ptr = ptof[i];
    while (ptr != NULL) {
      /* if the frame is dirty, write it out */
      if (ptr->dirty == 1) {
        dsMan.WritePage(ptr->page_id, buf[ptr->frame_id]);
      }
      /* Increment the ptr through the chain */
      ptr = ptr->next;
    }
  }
}

/* This function prints a part of the frame char field */
void BMgr::printValue(int frame_id, int offset, int len) {
  int i = 0;
  for (i = offset; i < (offset + len); i++) {
    if (buf[frame_id].field[i] == '\0') {
      cout << " ";
      return;
    }
    cout << buf[frame_id].field[i];
  }
}
