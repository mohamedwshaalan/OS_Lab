#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

struct student{
  char name[10000];
  long ID;
  float GPA;
  struct student* next;
};

void insert(struct student** student, char* name, long ID, float GPA){

  struct student* temp = (struct student*)malloc(sizeof(struct student));

  strcpy(temp->name, name);
  temp->GPA= GPA;
  temp->ID = ID;
  temp->next = *student;

  *student = temp;

}

bool validateID(long ID){
  while(ID!=0){
    if(ID%10>9 || ID%10<0){
      return false;
    }
    ID = ID/10;
  }
  return true;
}

void write_data(struct student* studentList, char* filename){

  FILE* fp = fopen(filename,"w+");
  if(fp==NULL){
    printf("File not found.\n");
    return;
  }
  while(studentList!=NULL){
    fprintf(fp,"%s %lu %0.3f\n", studentList->name, studentList->ID, studentList->GPA);
    studentList = studentList->next;
}
  fclose(fp);

}

struct student* get_data(char* filename){

 FILE* fp = fopen(filename,"r");
 if(fp==NULL){
    printf("File not found.\n");
    return 0;
  }
  struct student* temp = NULL;
  char name[10000];
  long ID  = 0;
	float GPA = 0.0f;
  while(fscanf(fp,"%s %lu %f ",name,&ID,&GPA)!=EOF){
      insert(&temp,name,ID,GPA);
  }
  fclose(fp);
  return temp;
}

void print_stats(struct student* studentList){

  float totalStudents = 0;
  float minimum = 1000000;
  float maximum = -1000000;
  float sum = 0.0f;
  struct student* copy = NULL;
  copy = studentList;
   if(studentList==NULL){
    printf("List does not contain students.\n");
    return;
   }
  while(studentList!=NULL){
    if(studentList->GPA>maximum){
      maximum = studentList->GPA;
    }
    if(studentList->GPA<minimum){
      minimum = studentList->GPA;
    }
    sum = sum + (studentList->GPA);
    studentList = studentList->next;
    totalStudents++;
  }
  float avgGpa = (sum/totalStudents);
  float temp = 0.0f;
  while(copy!=NULL){
    temp = temp + pow((copy->GPA -avgGpa ),2);
    copy = copy->next;
  }
  float stdDiv = 0;
  stdDiv = sqrt((temp/totalStudents));
  printf("Average: %0.5f   Standard Deviation: %0.5f    Maximum: %0.3f   Minimum: %0.3f",avgGpa, stdDiv, maximum, minimum);
}

void print_data(struct student* studentList){
  if(studentList==NULL){
    printf("List does not contain students.\n");
    return;
  }


  printf("Name \t\t ID \t\t GPA \n=======================================\n"); 
  while(studentList!=NULL){
    printf("%-10s \t %-10lu \t %-10f \n", studentList->name, studentList->ID, studentList->GPA);
    studentList = studentList->next;
  }

  
}

int main(void) {

  programStart: 
  printf("Choose one of the following choices\n==================================\n");
  printf("a. Read students' data\n");
  printf("b. Enter students' data\n");
  printf("c. Calculate statistics of students' data\n");
  printf("d. Exit the program\n");
  
  char choice;
  printf("Enter your choice: ");
  scanf("%c",&choice);
  switch(choice){
    case 'a':
    case 'b':
    case 'c':
    {
      char filename[10000];
      printf("Enter filename: ");
      scanf("%s",filename);
      switch(choice){
          case 'a':
          {
            struct student* temp =get_data(filename);
            print_data(temp);
            break;
          }
          case 'b':
          {
            choice = '1';
            struct student* temp = NULL;
            while(choice!='0'){
              char name[10000];
              long ID  = 0;
              float GPA = 0.0f;
              printf("Enter Student Name: ");
              scanf(" %[^\n]s",name);
              printf("Enter Student ID: ");
              scanf("%lu",&ID);
              printf("Enter Student GPA: ");
              scanf("%f",&GPA);
              while(GPA>4 || GPA<0){
                printf("Invalid GPA value, Enter GPA between 0 and 4: ");
                scanf("%f",&GPA);
              }
              insert(&temp,name,ID,GPA);
              printf("Enter 0 to stop data entry. Enter any other character to continue.\n");
              scanf(" %c",&choice);
            }
            write_data(temp,filename);
            break;
          }
          case 'c':
          {
            struct student* temp =get_data(filename);
            print_stats(temp);
            break;
          }

      }
    }
    case 'd':
    {
     programEnd: exit(0);
    }
  }
}
