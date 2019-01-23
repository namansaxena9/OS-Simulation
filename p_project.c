#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include<math.h>
#define smax 1000000
#define Size 1000
#define num 50

int main_memory[smax],signal=0;
int main_alloc[smax];


typedef struct
{
  int frame;
 struct list *next;
}list;  

list *lstart=NULL;
list *lend=NULL;

void linsert(int frame)
{
   list *temp=(list *)malloc(sizeof(list));
   temp->frame=frame;
   temp->next=NULL;
   
   if(lstart==NULL)
   {
     lstart=temp;
     lend=temp;
   }
   else
   {
      lend->next=temp;
      lend=lend->next;
   }
}

void ldelete(int frame)
{
    if(lstart!=NULL)
   {
     list *temp=lstart->next;
     list *temp2=lstart->next;
     
     if(lstart->frame==frame)
     {
        lstart=lstart->next;
     }
     
     while(temp!=NULL && temp->frame!=frame)
     {
        temp2=temp;
        temp=temp->next;  
     } 
    
    if(temp!=NULL)
    {
      if(temp==lend)
      lend=temp2;
      
      temp2->next=temp->next;
    }
  }     

}

sem_t lock,lock2,lock3;

int p_count=0;

time_t time1;

typedef struct 
{
  int pid;
  int size;
  int *paget;
  int npages;
  struct pcb *next;
    
}pcb;  

pcb *pstart=NULL;

pcb *assign(int pid,int size)
{
  pcb *result=(pcb *)malloc(sizeof(pcb));
  
  result->pid=pid;
  result->size=size;
  result->next=NULL;
  
  if(size%4==0)
  {
  result->npages=size/4;
  }
  else
  {
   result->npages=size/4+1;
  }

  result->paget=(int*)malloc(sizeof(int)*result->npages);
 
 return result;
}  
   
void insert(pcb *z)
{
  pcb *temp=pstart;

  
  if(temp==NULL)
  {
     pstart=z;
  }
  else
  {
     
     while(temp->next!=NULL)
     {
        temp=temp->next;
     }   
   
    temp->next=z;
  }
}

void delete(int pid)
{
   if(pstart!=NULL)
   {
     pcb *temp=pstart->next;
     pcb *temp2=pstart->next;
     
     if(pstart->pid==pid)
     {
        pstart=pstart->next;
     }
     
     while(temp!=NULL && temp->pid!=pid)
     {
        temp2=temp;
        temp=temp->next;  
     } 
    
    if(temp!=NULL)
    {
      temp2->next=temp->next;
    }
  }    
}

int memory_alloc(pcb *temp)  
{
   
   list *z=lstart;
   
   int count=0;
   
   while(z!=NULL)
   {
     count++;
     z=z->next;
   }
       
   if(count<temp->npages)
   {
     return 0;
   }  
  
   z=lstart;
   
   count=0;
   
   printf("%d",temp->npages);
    
   while(count<temp->npages)
   {
      int x=z->frame;
      main_memory[x]=1;
     temp->paget[count]=x;
     z=z->next;
     ldelete(x);
    count++;
   }         
   
 return 1;
}

void memory_dealloc(pcb *temp)
{
   int i;
   for(i=0;i<temp->npages;i++)
   {
     main_memory[temp->paget[i]]=-1;
     linsert(temp->paget[i]);
   }  
}   


void* lts()
{
   while(1)
 {

   sem_wait(&lock);
	if(signal==1)
	{
		sem_post(&lock2);
		pthread_exit(0);
	}
   int work=rand()%3;
   
   if(work==2)
   {
      
      int size=12+(rand()%20)*(rand()%20);
      pcb* temp=assign((int)time(&time1),size);
     
      if(memory_alloc(temp))
      {
        printf("\n ::NEW PROCESS CREATED:: \n");
        insert(temp);
      }
      else
      {
        printf("\n PROCESS CAN NOT BE CREATED ");
      }
             
   }        
   sem_post(&lock2);
 
  }

} 
  
    

void* mts()
{
   while(1)
  {
     sem_wait(&lock2);
	if(signal==1)
		{
		sem_post(&lock3);
		pthread_exit(0);
		}
     
     pcb *temp=pstart;
     
     while(temp!=NULL)
     {
        if((int)time(&time1)-temp->pid>10)
        {
            printf("\n::SUSPENDING PROCESS WITH PID %d :: \n",temp->pid);
            
            memory_dealloc(temp);
            
            delete((int)temp->pid); 
            break;
         }
       temp=temp->next;
     }
     
    sem_post(&lock3);
    
  }
}         
  
void* os()
{
 
  int last=(int)time(&time1);
  
  while(1)
  {
    sem_wait(&lock3);
     
     int current=(int)time(&time1);
     
     if(current-last>1)
     {
       last=current;
       system("clear");
       
       pcb *temp=pstart;
       
       int i=1;
       
       while(temp!=NULL)
       {
         printf("%d PID: %d  SIZE: %d (KB)   PAGES: %d \n",i,temp->pid,temp->size,temp->npages);  
         
         i++;
         temp=temp->next;
    
         if(i>6)
         break;
       }
       
       int choice;
       printf("\n ENTER THE PROCESS RANK(ENTER 0 TO CONTINUE OR -1 TO STOP)::  ");
       
       scanf("%d",&choice);
	if(choice==-1)
		{
			signal=1;
			sem_post(&lock);
			pthread_exit(0);
		}
       
       temp=pstart;
       
       for(i=0;i<choice-1;i++)
       {
         temp=temp->next;
       }  
        
       system("clear"); 
      if(choice!=0)
     {   
       for(i=0;i<temp->npages;i++)
      {
        printf(" PAGE %d :: FRAME %d \n",i+1,temp->paget[i]);
      }
      
      int m;
      scanf("%d",&m);
     } 
      system("clear");
     }
     
   sem_post(&lock);
 
  }
}          

void free_frame()
{
  int count=0;
  
   for(count=0;count<smax;count++) 
   { 
    // int work=rand()%smax;
     
      //  main_alloc[count]=1;
        main_memory[count]=-1;
        linsert(count);
      //  count++;
    }  
 
} 

void memManagement()
{
	sem_init(&lock,0,1);
  sem_init(&lock2,0,0);
  sem_init(&lock3,0,0);
  
  int i=0;
  for(i=0;i<smax;i++)
  {
    main_alloc[i]=0;
  }  
  
  free_frame();
 
  pthread_t tmain;
  pthread_t tid[2];
  
  pthread_create(&tmain,NULL,os,NULL);
  pthread_create(&tid[0],NULL,lts,NULL);
  pthread_create(&tid[1],NULL,mts,NULL); 
  pthread_join(tmain,NULL);
  pthread_join(tid[0],NULL);
  pthread_join(tid[1],NULL);
}
 

int *fileMem,**dirAT,*fileSize;

void *contiguos(void *arg)
{
	int i,j,x,s,avail;
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(2*sizeof(int));
	}
	for(i=0;i<num;i++)
	{
		Again:		
		x=rand()%1000;
		while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
		s=x;
		j=0;
		while(fileMem[x]==-1 && j<fileSize[i])
			{	
				x++;
				j++;
				x=x%1000;				
			}
		if(j==fileSize[i])
		{
			dirAT[i][0]=s;
			dirAT[i][1]=fileSize[i];
			for(j=s;j<s+fileSize[i];j++)
			{
				fileMem[j]=i;
			}
		}
		else
		{
			goto Again;
		}
	}
	
	FILE *fp;
	fp=fopen("contDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t%d\n",i,dirAT[i][0],dirAT[i][1]);
	}
	fclose(fp);

	free(dirAT);
}


void * linked(void *arg)
{
	int x,i,j,l,point[Size];
	for(i=0;i<Size;i++)
	{
		point[i]=-1;
	}
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(2*sizeof(int));
	}
	for(i=0;i<num;i++)
	{
		x=rand()%1000;
		while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
		l=x;
		dirAT[i][0]=x;
		j=1;
		while(j<fileSize[i])
		{
			x=rand()%1000;
			while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
			fileMem[x]=i;
			point[l]=x;
			l=x;
			j++;
		}
		dirAT[i][1]=l;
	}

	FILE *fp;
	fp=fopen("linkDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t%d\n",i,dirAT[i][0],dirAT[i][1]);
	}
	fclose(fp);

	free(dirAT);
}


void * indexed(void *arg)
{
	int i,j,x;
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(1*sizeof(int));
	}
	int indexList[num][8];
	for(i=0;i<num;i++)
	{
		for(j=0;j<8;j++)
		{
			indexList[i][j]=-1;
		}
	}
	for(i=0;i<num;i++)
	{
		dirAT[i][0]=i;
		for(j=0;j<fileSize[i];j++)
		{
			x=rand()%1000;
			while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
			indexList[i][j]=x;
			fileMem[x]=i;
		}
	}

	FILE *fp;
	fp=fopen("indDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t\t",i,dirAT[i][0]);
		for(j=0;j<8;j++)
		{
			fprintf(fp,"%d\t",indexList[i][j]);
		}
	fprintf(fp,"\n");
	}
	fclose(fp);

	free(dirAT);
}


void reset()
{
	int i=0;
	for(i=0;i<Size;i++)
	{
		fileMem[i]=-1;
	}
}

void fileManagement()
{
	pthread_t t[3];
	int i;

	fileMem=malloc(Size*sizeof(int));
	fileSize=malloc(num*sizeof(int));	
	
	reset();

	FILE *fp;
	fp=fopen("Files.txt","r");
	for(i=0;i<num;i++)
	{
		fscanf(fp,"%d",&fileSize[i]);
	}
	fclose(fp);

	while(1)
	{
	printf("\n\n\n\n\n");
	printf("\t\t\t\tFile Allocation Method :\n");
	printf("\t\t\t\t1. Contiguous Allocation\n");
	printf("\t\t\t\t2. Linked Allocation\n");
	printf("\t\t\t\t3. Indexed Allocation\n\n\n");
	int c;
	printf("Enter Your Choice : \t");
	scanf("%d",&c);
	

	if(c==1)
	{
	pthread_create(&t[0],NULL,contiguos,NULL);
	pthread_join(t[0],NULL);

	fp=fopen("Contiguous.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);
	
	system("gedit contDir.txt");	
	system("gedit Contiguous.txt");
	
	reset();
	}
	else if(c==2)
	{
	pthread_create(&t[1],NULL,linked,NULL);
	pthread_join(t[1],NULL);

	fp=fopen("Linked.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);

	system("gedit linkDir.txt");	
	system("gedit Linked.txt");

	reset();
	}
	else if(c==3)
	{
	pthread_create(&t[2],NULL,indexed,NULL);
	pthread_join(t[2],NULL);
	fp=fopen("Indexed.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);

	system("gedit indDir.txt");	
	system("gedit Indexed.txt");

	reset();
	}
	else
	{
		break;
	}
	}
}

int *req_que,req,cyl,start;

int absolute(int a,int b)
  {
     int c;
     c=a-b;
     if(c<0)
         return -c;
     else
         return c;
  }

void asc(int *a)
 {
   int i,j,x;
   for(i=0;i<req;i++)
     {
        for(j=0;j<req-i-1;j++)
          {   
            if(a[j]>a[j+1])
              {
                x=a[j];
                a[j]=a[j+1];
                a[j+1]=x;
               }
          }
     }
 }

void desc(int pos,int *a)
 {
   int i,j,x;
   for(i=0;i<pos;i++)
     {  
        for(j=0;j<pos-i-1;j++)
          {
	     if(a[j]<a[j+1])
               {
                 x=a[j];
                 a[j]=a[j+1];
                 a[j+1]=x;
               }
           }
      }
 }

void* fcfs(void *arg)
  {
    int k,*que=(int *)arg,count=0,count1=2,x=start,*res;
    res=(int *)malloc((req+2)*sizeof(int));
    res[1]=x;

    for(k=0;k<req;k++)
       {
          x-=que[k];	    
          if(x<0)
              x=-x;
          count+=x;
	  x=que[k];	    
          res[count1++]=x;	 
       }
     res[0]=count;

    pthread_exit((void *)res);
  }

void* sstf(void *arg)
  {
    int i,j,k,pos,min,*arr=(int *)arg,count=0,count1=2,x=start,*res;
    int que[req];
    for(k=0;k<req;k++)
      {
        que[k]=arr[k]; 
      }    
    res=(int *)malloc((req+2)*sizeof(int));
    res[1]=x;

    for(i=0;i<req;i++)
       {
          min=absolute(que[i],x);
          pos=i;
          for(j=i;j<req;j++)
             {
                if(min>absolute(x,que[j]))
                  {
                    pos=j;
                    min=absolute(x,que[j]);
                  }
             }
           count+=absolute(x,que[pos]);
           x=que[pos];
           que[pos]=que[i];
           que[i]=x;
           res[count1++]=x;
          
       }
      res[0]=count;

    pthread_exit((void *)res);       
  }

void* scan(void *arg)
  {
    int i,k,pos=0,min,*arr=(int *)arg,count=0,count1=2,x=start,*res;
    int que[req];
    for(k=0;k<req;k++)
      {
        que[k]=arr[k]; 
      }    
    res=(int *)malloc((req+3)*sizeof(int));
    res[1]=x;

    asc(que);
    for(i=0;i<req;i++)
       {
          if(que[i]<=start)
            {			    
              pos++;
            }
       }
    
    desc(pos,que);
    
    for(i=0;i<pos;i++)
      {
         count+=absolute(que[i],x);
         x=que[i];
         res[count1++]=x;
      }
    count+=absolute(x,0);
    x=0;
    res[count1++]=0;
    for(i=pos;i<req;i++)
      {
         count+=absolute(que[i],x);
         x=que[i];
         res[count1++]=x;
      }
    res[0]=count;
   
    pthread_exit((void *)res);
  }

void* cscan(void *arg)
  {//z=1
    int i,j,k,pos=0,min,*arr=(int *)arg,count=0,count1=2,x=start,*res;
    int que[req];
    for(k=0;k<req;k++)
      {
        que[k]=arr[k]; 
      }
    res=(int *)malloc((req+2)*sizeof(int));
    res[1]=x;

    asc(que);
    for(i=0;i<req;i++)
      {
         if(que[i]<start)
           {
	     pos++;
           }
      }
    for(i=pos;i<req;i++)
      {
         count+=absolute(x,que[i]);
         x=que[i];
         res[count1++]=x;
      }
    count+=absolute(cyl-1,x);
    x=0;
    res[count1++]=0;
    for(i=0;i<pos;i++)
      {
         count+=absolute(x,que[i]);
         x=que[i];
         res[count1++]=x;
      }
    res[0]=count;

   pthread_exit((void *)res); 
  }

void* look(void *arg)
  {//z=1
    int i,k,j,pos=0,min,*arr=(int *)arg,count=0,count1=2,x=start,*res;
    int que[req];
    for(k=0;k<req;k++)
      {
        que[k]=arr[k]; 
      }
    res=(int *)malloc((req+2)*sizeof(int));
    res[1]=x; 
    
    asc(que);
    for(i=0;i<req;i++)
      {
	 if(que[i]<=start)
           {
   	     pos++;
           } 
      }	    
    desc(pos,que);
    for(i=0;i<pos;i++)
      {
         count+=absolute(que[i],x);
         x=que[i];
         res[count1++]=x;
      }
    for(i=pos;i<req;i++)
      {
         count+=absolute(que[i],x);
         x=que[i];
         res[count1++]=x;
      }
    res[0]=count;

    pthread_exit((void *)res);
  }

void* clook(void *arg)
  {//z=1
    int i,k,j,pos=0,min,*arr=(int *)arg,count=0,count1=2,x=start,*res;
    int que[req];
    for(k=0;k<req;k++)
      {
        que[k]=arr[k]; 
      }    
    res=(int *)malloc((req+2)*sizeof(int));
    res[1]=x;
    asc(que);
    for(i=0;i<req;i++)
      {
          if(que[i]<start)
            {
	      pos++;
            }
      } 
    for(i=pos;i<req;i++)
      {
        count+=absolute(x,que[i]);
        x=que[i];
        res[count1++]=x;
      }
    for(i=0;i<pos;i++)
      {
         count+=absolute(x,que[i]);
         x=que[i];
         res[count1++]=x;
      }
     res[0]=count;

    pthread_exit((void *)res);
 }

void diskScheduling()
{
  int i,*r1,*r2,*r3,*r4,*r5,*r6,choice;
  pthread_t FCFS,SSTF,SCAN,CSCAN,LOOK,CLOOK;

  printf("\t\t**************DISK SCHEDULING***************");
  printf("\n\t\t\tEnter the number of Cylinders/Tracks : ");
  scanf("%d",&cyl);
  printf("\n\t\t\tEnter the number of requests : ");
  scanf("%d",&req);
  printf("\n\t\t\tEnter current position of read/write head: ");
  scanf("%d",&start);
  
  req_que=(int *)malloc(req*sizeof(int));

  printf("\n\t\t\tEnter the request queue : ");
      for(i=0;i<req;i++)
        {
          scanf("%d",&req_que[i]);
          if(req_que[i]>=cyl)
              {
                 printf("\n\t\t\tinvalid input");
                 scanf("%d",&req_que[i]);
              }
        }
  
  int *copy_que,j;
  copy_que=(int *)malloc(req*sizeof(int));
    for(j=0;j<req;j++)
        {
         copy_que[j]=req_que[j]; 
        }
  
  pthread_create(&FCFS,NULL,fcfs,(void *)copy_que);
  pthread_create(&SSTF,NULL,sstf,(void *)copy_que);
  pthread_create(&SCAN,NULL,scan,(void *)copy_que);
  pthread_create(&CSCAN,NULL,cscan,(void *)copy_que); 
  pthread_create(&LOOK,NULL,look,(void *)copy_que);
  pthread_create(&CLOOK,NULL,clook,(void *)copy_que);
  
  pthread_join(FCFS,(void **)&r1);
  pthread_join(SSTF,(void **)&r2);
  pthread_join(SCAN,(void **)&r3);
  pthread_join(CSCAN,(void **)&r4);
  pthread_join(LOOK,(void **)&r5);
  pthread_join(CLOOK,(void **)&r6);


do
   {
      printf("\n\nDISK SCHEDULING ALGORITHMS\n1. FCFS\n2. SSTF\n3. SCAN\n4. C-SCAN\n5. LOOK\n6. C-LOOK");
      printf("\nEnter no of choices : ");
      scanf("%d",&choice);
      int r,c[choice],minn,cmin;
      printf("\nEnter the choices : ");
      for(r=0;r<choice;r++)
        {
           scanf("%d",&c[r]);
        }
      printf("\n\nREQUESTED QUEUE IS:");
      for(i=0;i<req;i++)
        {
           printf("\t%d", copy_que[i]);
        }
     for(r=0;r<choice;r++)
      {
        switch(c[r])
         {
           case 1:printf("\n\n\t\t***************FCFS****************\n\n");
                  printf("\nTotal Head Movement : %d Cylinders\n\n",r1[0]);
                  printf("Scheduling services the request in the order that follows:\n");
                  int f1;
                  for (f1=1;f1<(req+2);f1++)
                    {
                      printf("%d ",r1[f1]);
                    }
                  
                  FILE *fp1;
                  
                  fp1=fopen("fcfs.dat","w");
                  if(fp1 == NULL)
                    {
                      printf("unable to open file");
                      return;
                    }
                  
                  for (f1=1;f1<(req+2);f1++)
                    {
                      fprintf(fp1,"%d %d\n",f1,r1[f1]);
                    }
                  fclose(fp1);

                  FILE *pipe_gp1 = popen("gnuplot -p", "w");
                  //fputs("set xlabel \n",pipe_gp1);
                  fputs("set border linewidth 1.5\n",pipe_gp1);
                  fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp1);
                  fputs("set xrange [-2:7]\n",pipe_gp1);
                  fputs("set yrange [-2:7]\n",pipe_gp1);
                  fputs("plot  'fcfs.dat' with linespoints ls 1\n", pipe_gp1);
                  pclose(pipe_gp1);
                  

            	  break;

          case 2:printf("\n\n\t\t***************SSTF***************\n\n");
                 printf("\nTotal Head Movement: %d Cylinders\n\n",r2[0]);
                 printf("Scheduling services the request in the order that follows:\n");
                 int f2;
                 for (f2=1;f2<(req+2);f2++)
                    {
                      printf("%d ",r2[f2]);
                    }                                   

                 FILE *fp2;
          
                 fp2=fopen("sstf.dat","w");
                 if(fp2 == NULL)
                   {
                      printf("unable to open file");
                      return;
                   }
                 
                 for (f2=1;f2<(req+2);f2++)
                   {
                     fprintf(fp2,"%d %d\n",f2,r2[f2]);
                   }
                 fclose(fp2);

                 FILE *pipe_gp2 = popen("gnuplot -p", "w");
                 //fputs("set xlabel \n",pipe_gp2);
                 fputs("set border linewidth 1.5\n",pipe_gp2);
                 fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp2);
                 fputs("set xrange [-2:7]\n",pipe_gp2);
                 fputs("set yrange [-2:7]\n",pipe_gp2);
                 fputs("plot  'sstf.dat' with linespoints ls 1\n", pipe_gp2);
                 pclose(pipe_gp2);

	         break;

          case 3:printf("\n\n\t\t***************SCAN**************\n\n");
                 printf("\nTotal Head Movement: %d Cylinders\n\n",r3[0]);
                 printf("Scheduling services the request in the order that follows:\n");
                 int f3;
                 for (f3=1;f3<(req+3);f3++)
                   {
                     printf("%d ",r3[f3]);
                   }

                 FILE *fp3;
          
                 fp3=fopen("scan.dat","w");
                 if(fp3 == NULL)
                   {
                      printf("unable to open file");
                      return;
                   }
                 
                 for (f3=1;f3<(req+3);f3++)
                   {
                     fprintf(fp3,"%d %d\n",f3,r3[f3]);
                   }
                 fclose(fp3);

                 FILE *pipe_gp3 = popen("gnuplot -p", "w");
                 //fputs("set xlabel \n",pipe_gp3);
                 fputs("set border linewidth 1.5\n",pipe_gp3);
                 fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp3);
                 fputs("set xrange [-2:7]\n",pipe_gp3);
                 fputs("set yrange [-2:7]\n",pipe_gp3);
                 fputs("plot  'scan.dat' with linespoints ls 1\n", pipe_gp3);
                 pclose(pipe_gp3);
                        
                 break;

          case 4:printf("\n\n\t\t***************C-SCAN************\n\n");
                 printf("\nTotal Head movement: %d Cylinders\n\n",r4[0]);
                 printf("Scheduling Services the request in the order that follows:\n");
                 int f4;
                 for (f4=1;f4<(req+3);f4++)
                   {
                     printf("%d ",r4[f4]);
                   }

                 FILE *fp4;
          
                 fp4=fopen("cscan.dat","w");
                 if(fp4 == NULL)
                   {
                      printf("unable to open file");
                      return;
                   }
                 
                 for (f4=1;f4<(req+3);f4++)
                   {
                     fprintf(fp4,"%d %d\n",f4,r4[f4]);
                   }
                 fclose(fp4);

                 FILE *pipe_gp4 = popen("gnuplot -p", "w");
                 //fputs("set xlabel \n",pipe_gp4);
                 fputs("set border linewidth 1.5\n",pipe_gp4);
                 fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp4);
                 fputs("set xrange [-2:7]\n",pipe_gp4);
                 fputs("set yrange [-2:7]\n",pipe_gp4);
                 fputs("plot  'cscan.dat' with linespoints ls 1\n", pipe_gp4);
                 pclose(pipe_gp4);                
      
                 break;

          case 5:printf("\n\n\t\t*****************LOOK**********\n\n");
                 printf("\nToal Head Movement: %d Cylinders\n\n",r5[0]);
                 printf("\nScheduling services the request in the order as follows :\n");
                 int f5;
                 for (f5=1;f5<(req+2);f5++)
                   {
                     printf("%d ",r5[f5]);
                   }

                 FILE *fp5;
          
                 fp5=fopen("look.dat","w");
                 if(fp5 == NULL)
                   {
                      printf("unable to open file");
                      return;
                   }
                 
                 for (f5=1;f5<(req+2);f5++)
                   {
                     fprintf(fp5,"%d %d\n",f5,r5[f5]);
                   }
                 fclose(fp5);

                 FILE *pipe_gp5 = popen("gnuplot -p", "w");
                 //fputs("set xlabel \n",pipe_gp5);
                 fputs("set border linewidth 1.5\n",pipe_gp5);
                 fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp5);
                 fputs("set xrange [-2:7]\n",pipe_gp5);
                 fputs("set yrange [-2:7]\n",pipe_gp5);
                 fputs("plot  'look.dat' with linespoints ls 1\n", pipe_gp5);
                 pclose(pipe_gp5);
                 
                 break;

          case 6:printf("\n\n\t\t***************C-LOOK***********\n\n");
                 printf("\nToal Head Movement: %d Cylinders\n\n",r6[0]);
                 printf("\nScheduling services the request in the order as follows :\n");
                 int f6;
                 for (f6=1;f6<(req+2);f6++)
                   {
                     printf("%d ",r6[f6]);
                   }

                 FILE *fp6;
          
                 fp6=fopen("clook.dat","w");
                 if(fp6 == NULL)
                   {
                      printf("unable to open file");
                      return;
                   }
                 
                 for (f6=1;f6<(req+2);f6++)
                   {
                     fprintf(fp6,"%d %d\n",f6,r6[f6]);
                   }
                 fclose(fp6);

                 FILE *pipe_gp6 = popen("gnuplot -p", "w");
                 //fputs("set xlabel \n",pipe_gp6);
                 fputs("set border linewidth 1.5\n",pipe_gp6);
                 fputs("set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 2\n",pipe_gp6);
                 fputs("set xrange [-2:7]\n",pipe_gp6);
                 fputs("set yrange [-2:7]\n",pipe_gp6);
                 fputs("plot  'clook.dat' with linespoints ls 1\n", pipe_gp6);
                 pclose(pipe_gp6);			   
                 break;

         }
      }

        printf("\n\nDo you want to continue(1 to continue) :");
        scanf("%d",&choice);
 }while(choice==1);
}






int main()
{

	while(1)
	{
  	printf("\n\n\n\n\n");
	printf("\t\t\t\tWelcome to OS project:\n");
	printf("\t\t\t\t1. Disk Scheduling\n");
	printf("\t\t\t\t2. Memory Management\n");
	printf("\t\t\t\t3. File Management\n\n\n");
	int c;
	printf("\t\t\t\tEnter Your Choice : \t");
	scanf("%d",&c);
	if(c==1)
	{
		diskScheduling();
	}
	else if(c==2)
	{
		memManagement();
	}
	else if(c==3)
	{
		system("cc file.c -lpthread");
		system("./a.out");
	}
	else
	{
		break;
	}
	}
return 0;
}
