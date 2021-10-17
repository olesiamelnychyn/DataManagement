//  DSA_zadanie_1

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void *memory_alloc(unsigned int size);
int memory_free(void *valid_ptr);
int memory_check(void *ptr);
void memory_init(void *ptr, unsigned int size);

int* set_powers (int *powers);
void print_array(char *array, int size);
void set_pointers(unsigned char *ptr, int back, int size);
void addblock(int size, int best_fit);
void swap_pointers(int previous, int next);

unsigned char *Pointer;

//struct for head
typedef struct{
    short size;
    bool free;
} mem_block;

void memory_init(void *ptr, unsigned int size){
   
    Pointer= ptr; //save the array pointer in global variable
       
    for(int i=0; i<size; i++){
        *(Pointer+i)=0;
    }
       
    int powers[14];         //array of sizes of ranges
    set_powers(powers);
    int number_of_lists=0;
       
    for(int i=13; i>=0; i--){          //founding out how many list will be
        if((int)size-powers[i]-(i+1)*2-8==0){
            number_of_lists=i+1;
            break;
        } else if((int)size-powers[i]-(i+1)*2-8>0){
            number_of_lists=i+2;
            break;
        }
    }
       
    *(Pointer)=number_of_lists; //first variable shows us how many pointers at the lists are, and also can help to found out where the memory, that we can provide, starts
    *(Pointer+size-1)=0; //setting the end of the memory
       
    short i=0;
    for( i=0; i<14; i++){              //founding out the list which may have enough memory
        if((int)size-powers[i]-number_of_lists*2-8<=0){
            i++;
            break;
        }
    }
       
    int size_free = size - number_of_lists * 2 - 8; // size of memory we can provide, without pointers, and heads of ranges (real memory)
      
    mem_block head; //creating head
    head.free = 1;
    head.size = size_free;

    *(mem_block *)(Pointer + number_of_lists*2 + 1) = head; //puting heads to the real memory and, which means its size and is it free or not
    *(mem_block *)(Pointer+size-4) = head;

    *(short*)(Pointer+number_of_lists*2+1+3+2)=i*2-1; //adding pointer at the place where is pointer at this range
    *(Pointer+i*2-1)=number_of_lists*2+1; //ading pointer at the real memory
}

void *memory_alloc(unsigned int size){
    
    int powers[14];         //array of sizes of ranges
    set_powers(powers);

    int power=0;
    for( power=0; power<14; power++){  //founding out the list which may have enough memory for us
        if((int)size-powers[power]<=0){
            power++;
            break;
        }
    }
    
    int list_number=0;
    for (int j=power*2-1; j<*(Pointer)*2+1; j+=2) {
        if(*(short*)(Pointer+j)>0){
            list_number=j;
            break;
        }
    }

    if (list_number==0) { //if there is no free memory, big enough
        return NULL;
    } else {
        int first_come_best=0;
            
        while(1){ //going through all the lists
            if(*(short*)(Pointer+list_number)==0){ //if there is no nodes in the list we jump to the next one
                list_number+=2;
                continue;
            }
                
            if(list_number==*(Pointer)*2+1 && first_come_best!=0){ //if there is no more list, but there is a block we can provide, but not exactly fit (is bigger and we cannot diviode it in to two)
                int best_fit = first_come_best;
                mem_block bff= *(mem_block*)(Pointer+best_fit);
                size=bff.size;
                short previous =*(short*)(Pointer+best_fit+5);
                short next=*(short*)(Pointer+best_fit+3);
                swap_pointers(previous, next);
                addblock(size, best_fit);
                return Pointer+best_fit+3;
            } else if(list_number==*(Pointer)*2+1 && first_come_best==0){ //if there is no more list and no blocks to provide
                return NULL;
            }
               
            int current_place=*(short*)(Pointer+list_number); //where we are within the array(memory)
            int best_fit=current_place;
            int best_size=powers[list_number-1]+1; //remembering what range fits the best by size
            mem_block now = *(mem_block*)(Pointer+current_place);
            if(*(short*)(Pointer+current_place+3)==0 && now.size<size){
                list_number+=2;
                continue;
            } else if(*(short*)(Pointer+current_place+3)==0 && now.size==size){ //if we are lucky and the first one fits exactly
                *(short*)(Pointer+*(short*)(Pointer+current_place+5))=0;
                addblock(size, current_place);
                return Pointer+current_place+3;
            }

            while(*(short*)(Pointer+current_place+3)!=0){ //if there is a pointer at the next node
                now = *(mem_block*)(Pointer+current_place);
                
                if(now.size==(int)size){ //when the size of range is equal to the size of memory we need to provide
                    short previous =*(short*)(Pointer+current_place+5);
                    short next=*(short*)(Pointer+current_place+3);
                    swap_pointers(previous, next);
                    addblock(size, current_place);
                    return Pointer+current_place+3;
                } else {
                    if((now.size<best_size) && (now.size)>(int) size){ //trying to find the best fitted node
                        best_size=now.size;
                        best_fit=current_place;
                    }
                    current_place=*(short*)(Pointer+current_place+3);
                }
            }

            mem_block a= *(mem_block*)(Pointer+best_fit);
            
            if(a.size-(int)size-6<4){ //if there leaves not anough place to create a new range
                first_come_best=best_fit;
                list_number+=2;
                continue;
            } else{ //if there leaves anough place to create a new range
                addblock(size, best_fit);
                
                mem_block new;   //creating new block ehich is free
                new.free=1;
                new.size = a.size-(int)size-6;
                
                char y=*(Pointer+best_fit +(int)size+9);
                *(mem_block*)(Pointer+best_fit+(int)size+6)=new;
                *(Pointer+best_fit +(int)size+9)=y;
                
                y=*(Pointer+best_fit +a.size+6);
                *(mem_block*)(Pointer+best_fit +a.size+3)=new;
                *(Pointer+best_fit +a.size+6)=y;

                //searching to what list to add a new created range
                if(new.size<powers[(list_number-1)/2] && new.size>powers[(list_number-1)/2-1]){//if it belongs to the same list, as a mother one
                    
                    if(*(short*)(Pointer+best_fit+5)<(*Pointer)*2+1){
                        *(short*)(Pointer+*(short*)(Pointer+best_fit+5))+=(int)size+6;
                    } else{
                        *(short*)(Pointer+*(short*)(Pointer+best_fit+5) +3)+=(int)size+6;
                    }
                    
                    if(*(short*)(Pointer+best_fit+3) != 0){
                        *(short*)(Pointer+*(short*)(Pointer+best_fit+3) +5)+=(int ) size+6;
                    }
                    
                    if(*(short*)(Pointer+best_fit+3)!=0)
                        *(short*)(Pointer+best_fit+(int)size+6+3) = *(short*)(Pointer+best_fit+3);
                    *(short*)(Pointer+best_fit+(int)size+6+5)=*(short*)(Pointer+best_fit+5);
                    
                } else{ // if it belongs to another list
                    int k=0;
                    for (k=(list_number-1)/2-1; k>=0;k--) {
                        if(new.size-powers[k]==0){
                            k++;
                            break;
                        } else if ((new.size-powers[k]>0)){
                            k+=1;
                            break;
                        }
                    }
                   
                    k=k*2+1;
                    short previous =*(short*)(Pointer+best_fit+5);
                    short next=*(short*)(Pointer+best_fit+3);
                    swap_pointers(previous, next);

                    if(*(short*)(Pointer+k)==0){ //if there is no nodes in the list, where new one belongs
                        *(short*)(Pointer+k)=best_fit+(int)size+6;
                        *(short*)(Pointer+best_fit+(int)size+11)=k; // add pointer in the new node at the previous
                    } else{  //if there are nodes in the list, where new one belongs
                        short now = *(short*)(Pointer+k);
                        while (*(short*)(Pointer+now+3)!=0) {
                            now=*(short*)(Pointer+now+3);
                        }
                        *(short*)(Pointer+now+3)=best_fit+(int)size+6;
                        *(short*)(Pointer+best_fit+(int)size+11)=now;
                    }
                   
                }
                
            }
            return Pointer+best_fit+3;
        }
    }
}

int memory_free(void *valid_ptr){
    
    if(!memory_check(valid_ptr)){  //cheking if the pointer exist within our memory
        printf("Does not exists");
        return 1;
    }
    
    unsigned char *ptr;
    ptr=valid_ptr;
    mem_block this =*(mem_block*)(ptr-3);
    //unioning the nodes, to bigger ones
    int forward=this.size+3;
    mem_block forw=*(mem_block*)(ptr+forward);
    if (forw.free && *(ptr+forward)!=0){  //with next ones
        short previous =*(short*)(ptr+forward+5);
        short next=*(short*)(ptr+forward+3);
        swap_pointers(previous, next);
        forward+=forw.size+3;
    } else{
        forward=this.size;
    }
    
    int back=6;
    mem_block before =*(mem_block*)(ptr-back);
    if ((before.free) && *(ptr-back)!=*(Pointer+*Pointer*2-2)){  //with privious ones
        back+=before.size+3;
        short previous =*(short*)(ptr-back+5);
        short next=*(short*)(ptr-back+3);
        swap_pointers(previous, next);
    } else{
        back=3;
    }
    
    for(int i=-back+3;i<forward; i++){//seting "0" to the memory to better orienting
           *(ptr+i)=0;
       }
    
    if(forward==this.size && back==3){ // if there is no free nodes in the vicinity
        this.free = 1;
        *(mem_block*)(ptr-3)=this;  //just setting that it is free
        *(ptr)=0;
        int a =*(ptr+this.size+3);
        *(mem_block*)(ptr+this.size)=this;
        *(ptr+this.size+3)=a;
        set_pointers(ptr, 3, this.size);

    }
    else { //when there is node/-s in the vicinity
        int size_new=back+forward-3; //after unionig the size becomes bigger
        mem_block new;
        new.free=1;
        new.size = size_new;
        *(mem_block*)(ptr-back)=new;//setting heads at the new borders
        *(ptr-back+3)=0;
        int a =*(ptr+forward+3);
        *(mem_block*)(ptr+forward)=new;
        *(ptr+forward+3)=a;
        set_pointers(ptr, back, size_new);
    }
    return 0;
}

int memory_check(void *ptr){
    unsigned char *ptr1;
    ptr1=ptr;
    int a=*(Pointer)*2+1;         //index where memory we provide begins
    int i=0;

    while((Pointer+a)!=0){
        if((Pointer+a+3)==ptr1){
            i++;
            break;
        }
        mem_block b = *(mem_block*)(Pointer+a);
        a+=b.size+6;
    }
    if(i){
        return 1;
    } else
        return 0;
}

int* set_powers (int *powers){
    powers[0]=4;
    for (int i=1; i<14; i++) {
        powers[i]=powers[i-1]*2;
    }
    return powers;
}

void set_pointers(unsigned char *ptr, int back, int size){
    int powers[14];         //array of sizes of ranges
    set_powers(powers);
    int i=0;
    while(Pointer!=ptr-back-i){  //founding out at what index this node begins
        i++;
    }
    int j=0;
    while(size-powers[j]>0){ //founding out to what list add the node
        j++;
    }

    if(*(Pointer+j*2+1)!=0){ // if there already is/are node/nodes in the appropriate list
        int a=*(short*)(Pointer+j*2+1);
        while(*(short*)(Pointer+a+3)!=0){
            a =*(short*)(Pointer+a+3);
        }
        *(short*)(Pointer+a+3)=i;
        *(short*)(ptr-back+5)=a;
    } else{ //if the appropriate list is free
        *(short*)(ptr-back+5)=j*2+1;
        *(short*)(Pointer+j*2+1)=i;
    }
}

void addblock (int size, int best_fit){
    mem_block this;
    this.free = 0;
    this.size = size;
    
    int x=*(Pointer+best_fit+3);
    *(mem_block*)(Pointer+best_fit)=this; //adding head to provided memory
    *(Pointer+best_fit+3)=x;
    
     x=*(Pointer+best_fit+(int)size+6);
    *(mem_block*)(Pointer+best_fit+(int)size+3)=this;
    *(Pointer+best_fit+(int)size+6)=x;
}

void swap_pointers(int previous, int next){
    if(previous > *Pointer*2){
        *(short*)(Pointer+previous+3)=next;
    } else{
        *(short*)(Pointer+previous)=next;
    }
    if (next!=0)
        *(short*)(Pointer+next+5)=previous;
}

void print_array(char *array, int size){
    for(int i=0; i<size; i++){
        printf("%d  -  ", array[i]);
    }
    printf("\n\n\n");
}

//test 1
//int main(int argc, const char * argv[]) {
//
//
//    char region[100];
//    memory_init(region, 100);
//    print_array(region, 100);
//
//    char* pointer = (char*) memory_alloc(8);
//    if (pointer)
//            memset(pointer, 1, 8);
//    print_array(region, 100);
//
//    char* pointer1 = (char*) memory_alloc(8);
//    if (pointer1)
//         memset(pointer1, 2, 8);
//    print_array(region, 100);
//
//    char* pointer2 = (char*) memory_alloc(8);
//    if (pointer2)
//        memset(pointer2, 3, 8);
//    print_array(region, 100);
//
//    char* pointer4 = (char*) memory_alloc(8);
//    if (pointer4)
//            memset(pointer4, 4, 11);
//    print_array(region, 100);
//
//    char* pointer5 = (char*) memory_alloc(8);
//    if (pointer5)
//         memset(pointer5, 5, 8);
//    print_array(region, 100);
//
//    char* pointer6 = (char*) memory_alloc(8);
//    if (pointer6)
//        memset(pointer6, 6, 8);
//    print_array(region, 100);
//
//     if (pointer)
//         memory_free(pointer);
//    print_array(region, 100);
//
//    if (pointer5)
//        memory_free(pointer5);
//    print_array(region, 100);
//
//    if (pointer6)
//        memory_free(pointer6);
//    print_array(region, 100);
//
//
//    char* pointer7 = (char*) memory_alloc(8);
//    if (pointer7){
//        printf("Yes 7\n");
//         memset(pointer7, 7, 8);
//    }
//
//       print_array(region, 100);
//
//    char* pointer8 = (char*) memory_alloc(8);
//    if (pointer8){
//        printf("Yes 8\n");
//        memset(pointer8, 8, 8);
//    }
//
//        print_array(region, 100);
//    char* pointer9 = (char*) memory_alloc(8);
//     if (pointer9){
//         printf("Yes 9\n");
//         memset(pointer9, 8, 8);
//     }
//    print_array(region, 100);
//
////    if (pointer5)
////         memory_free(pointer5);
////    print_array(region, 100);
//
//
//    return 0;
//}

//test 2
//int main(int argc, const char * argv[]) {
//    char region[60];
//    memory_init(region, 60);
//    print_array(region, 60);
//    char* pointer = (char*) memory_alloc(30);
//    if (pointer)
//        memset(pointer, 1, 30);
//    print_array(region, 60);
//
//    if (pointer)
//           memory_free(pointer);
//       print_array(region, 60);
//
//    char* pointer1 = (char*) memory_alloc(38);
//    if (pointer1)
//        memset(pointer1, 2, 38);
//    print_array(region, 60);
//
//    if (pointer1)
//        memory_free(pointer1);
//    print_array(region, 60);
//    return 0;
//  }

//test 3
//int main(int argc, const char * argv[]) {
//    char region[100];
//    memory_init(region, 100);
//    print_array(region, 100);
//
//    char* pointer = (char*) memory_alloc(12);
//    if (pointer)
//        memset(pointer, 1, 12);
//    print_array(region, 100);
//
//    char* pointer1 = (char*) memory_alloc(23);
//    if (pointer1)
//        memset(pointer1, 2, 23);
//    print_array(region, 100);
//
//    char* pointer2 = (char*) memory_alloc(30);
//    if (pointer2)
//        memset(pointer2, 3, 30);
//    print_array(region, 100);
//
//    char* pointer3 = (char*) memory_alloc(19);
//    if (pointer3)
//        memset(pointer3, 4, 19);
//    print_array(region, 100);
//
//    if (pointer)
//        memory_free(pointer);
//    print_array(region, 100);
//
//    if (pointer2)
//         memory_free(pointer2);
//    print_array(region, 100);
//
//    if (pointer3)
//        memory_free(pointer3);
//    print_array(region, 100);
//
//    if (pointer1)
//        memory_free(pointer1);
//    print_array(region, 100);
//
//    return 0;
// }

//test 4
//int main(int argc, const char * argv[]) {
//
//    char region[40];
//    memory_init(region, 40);
//    print_array(region, 40);
//
//    char* pointer = (char*) memory_alloc(7);
//    if (pointer)
//            memset(pointer, 1, 7);
//    print_array(region, 40);
//
//    char* pointer1 = (char*) memory_alloc(11);
//    if (pointer1)
//         memset(pointer1, 2, 11);
//    print_array(region, 40);
//
//    char* pointer2 = (char*) memory_alloc(8);
//    if (pointer2)
//        memset(pointer2, 3, 8);
//    print_array(region, 40);
//
//    if (pointer)
//        memory_free(pointer);
//    print_array(region, 40);
//
//    if (pointer2)
//         memory_free(pointer2);
//    print_array(region, 40);
//
//    if (pointer1)
//        memory_free(pointer1);
//    print_array(region, 40);
//    return 0;
//}

//test 5
//int main(int argc, const char * argv[]) {
//    char region[500];
//    memory_init(region, 500);
//    print_array(region, 500);
//
//    char* pointer1 = (char*) memory_alloc(23);
//    if (pointer1)
//            memset(pointer1, 1, 23);
//    print_array(region, 500);
//
//    char* pointer2 = (char*) memory_alloc(23);
//    if (pointer2)
//         memset(pointer2, 2, 23);
//    print_array(region, 500);
//
//    char* pointer3 = (char*) memory_alloc(10);
//    if (pointer3)
//        memset(pointer3, 3, 10);
//    print_array(region, 500);
//
//    char* pointer4 = (char*) memory_alloc(21);
//    if (pointer4)
//        memset(pointer4, 1, 21);
//    print_array(region, 500);
//
//    char* pointer5 = (char*) memory_alloc(19);
//    if (pointer5)
//        memset(pointer5, 2, 19);
//    print_array(region, 500);
//
//    char* pointer6 = (char*) memory_alloc(11);
//    if (pointer6)
//        memset(pointer6, 3, 11);
//    print_array(region, 500);
//
//    char* pointer7 = (char*) memory_alloc(13);
//    if (pointer7)
//        memset(pointer7, 1, 13);
//    print_array(region, 500);
//
//    char* pointer8 = (char*) memory_alloc(17);
//    if (pointer8)
//        memset(pointer8, 2, 17);
//    print_array(region, 500);
//
//    char* pointer9 = (char*) memory_alloc(13);
//    if (pointer9)
//        memset(pointer9, 3, 13);
//    print_array(region, 500);
//
//    char* pointer10 = (char*) memory_alloc(18);
//    if (pointer10)
//        memset(pointer10, 1, 18);
//    print_array(region, 500);
//
//    char* pointer11 = (char*) memory_alloc(12);
//    if (pointer11)
//        memset(pointer11, 2, 12);
//    print_array(region, 500);
//
//    char* pointer12 = (char*) memory_alloc(19);
//    if (pointer12)
//        memset(pointer12, 3, 19);
//    print_array(region, 500);
//
//    char* pointer13 = (char*) memory_alloc(23);
//    if (pointer13)
//        memset(pointer13, 3, 23);
//    print_array(region, 500);
//
//    if (pointer6)
//        memory_free(pointer6);
//    print_array(region, 500);
//
//    if (pointer3)
//         memory_free(pointer3);
//    print_array(region, 500);
//
//    if (pointer11)
//        memory_free(pointer11);
//    print_array(region, 500);
//
//    if (pointer4)
//        memory_free(pointer4);
//    print_array(region, 500);
//
//    if (pointer10)
//        memory_free(pointer10);
//    print_array(region, 500);
//
//    if (pointer12)
//        memory_free(pointer12);
//    print_array(region, 500);
//
//    if (pointer7)
//        memory_free(pointer7);
//    print_array(region, 500);
//
//    if (pointer1)
//        memory_free(pointer1);
//    print_array(region, 500);
//
//    if (pointer2)
//        memory_free(pointer2);
//    print_array(region, 500);
//
//
//    if (pointer8)
//        memory_free(pointer8);
//    print_array(region, 500);
//
//    if (pointer9)
//        memory_free(pointer9);
//    print_array(region, 500);
//
//    char* pointer14 = (char*) memory_alloc(19);
//    if (pointer14)
//        memset(pointer14, 3, 19);
//    print_array(region, 500);
//
//    char* pointer15 = (char*) memory_alloc(23);
//    if (pointer15)
//        memset(pointer15, 3, 23);
//    print_array(region, 500);
//
//    char* pointer16 = (char*) memory_alloc(19);
//    if (pointer16)
//        memset(pointer16, 3, 19);
//    print_array(region, 500);
//
//    char* pointer17 = (char*) memory_alloc(23);
//    if (pointer17)
//        memset(pointer17, 3, 23);
//    print_array(region, 500);
//
//    if (pointer13)
//        memory_free(pointer13);
//    print_array(region, 500);
//
//    if (pointer5)
//        memory_free(pointer5);
//    print_array(region, 500);
//
//    if (pointer17)
//        memory_free(pointer17);
//    print_array(region, 500);
//
//    if (pointer15)
//        memory_free(pointer15);
//    print_array(region, 500);
//
//    if (pointer14)
//        memory_free(pointer14);
//    print_array(region, 500);
//
//    if (pointer16)
//        memory_free(pointer16);
//    print_array(region, 500);
//
//    return 0;
//}

//test 5
//int main(int argc, const char * argv[]) {
//    int n=60;
//    char region[n];
//    memory_init(region, n);
//    print_array(region, n);
//
//    char* pointer = (char*) memory_alloc(8);
//    if (pointer)
//            memset(pointer, 1, 8);
//   print_array(region, n);
//
//    char* pointer1 = (char*) memory_alloc(8);
//    if (pointer1)
//         memset(pointer1, 2, 8);
//    print_array(region, n);
//
//    char* pointer2 = (char*) memory_alloc(8);
//    if (pointer2)
//        memset(pointer2, 3, 8);
//    print_array(region, n);
//
//    if (pointer)
//        memory_free(pointer);
//    print_array(region, n);
//
//    if (pointer1)
//         memory_free(pointer1);
//    print_array(region, n);
//
//    if (pointer2)
//        memory_free(pointer2);
//    print_array(region, n);
//    return 0;
//}



//int main(int argc, const char * argv[]) {
//    char region[50];
//    memory_init(region, 50);
//    print_array(region, 50);
//
//    char* pointer = (char*) memory_alloc(8);
//    if (pointer)
//            memset(pointer, 1, 8);
//   print_array(region, 50);
//
//    char* pointer1 = (char*) memory_alloc(12);
//    if (pointer1)
//         memset(pointer1, 2, 12);
//    print_array(region, 50);
//
//    char* pointer2 = (char*) memory_alloc(8);
//    if (pointer2)
//        memset(pointer2, 3, 8);
//    print_array(region, 50);
//
//    char* pointer3 = (char*) memory_alloc(12);
//       if (pointer3)
//           memset(pointer3, 3, 12);
//       print_array(region, 50);
//
//    if (pointer)
//        memory_free(pointer);
//    print_array(region,50);
//
//    if (pointer1)
//         memory_free(pointer1);
//    print_array(region, 50);
//
//    if (pointer2)
//        memory_free(pointer2);
//    print_array(region, 50);
//
//    if (pointer3)
//           memory_free(pointer3);
//       print_array(region, 50);
//    return 0;
//}

int main(int argc, const char * argv[]) {
    char region[50];
    memory_init(region, 50);
    print_array(region, 50);

    char* pointer = (char*) memory_alloc(8);
    if (pointer)
            memset(pointer, 1, 8);
   print_array(region, 50);

    char* pointer1 = (char*) memory_alloc(12);
    if (pointer1)
         memset(pointer1, 2, 12);
    print_array(region, 50);
    
    if (pointer)
        memory_free(pointer);
    print_array(region,50);

    char* pointer2 = (char*) memory_alloc(8);
    if (pointer2)
        memset(pointer2, 3, 8);
    print_array(region, 50);
    return 0;
}

