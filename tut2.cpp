// type casting  in C++
#include<iostream>
using namespace std;

//TYPE CASTING ----------->>  convert one data-type to another data-type 

int main (){

    

// one is type conversion ----->> it is implicit   compiler convert one data type to another    
             char grade = 's';
             int value = grade;
             cout<<value<<endl;

// another is type casting ----->>  it is explicit    programmer convert one data type to another            
            double price =3454.34;
            int gheeprice = (int)price;
            cout<<gheeprice<<endl;

// input in c++
int age;
cout<<"Enter your age";
cin>>age;
cout<<"your age is" << age <<endl;

    return 0;
}