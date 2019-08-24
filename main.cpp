//-----------------------------------------------------------------------------
/*!
   \file
   \brief A simple Observable implementation.

   Let’s write our own Observable interface implementation to understand what’s going on under the hood when we work with RxJS.

   An observable is just a function. This function takes in an observer as an argument, and returns a subscription object.

   An observer is just an object with three methods: next which takes in a value, error which takes in an error message and
   complete with has no arguments.

   A subscription object represents a disposable resource, such as the execution of an Observable. This subscription has a
   bunch of methods such as add and remove, but the most important one is unsubscribe which takes no argument and just disposes
   the resource held by the subscription.

   Nach: https://medium.com/@fknussel/a-simple-observable-implementation-c9c809c89c69
*/
//-----------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>
#include <iostream>
#include <string>


/* -- Defines ------------------------------------------------------------- */
using std::cout;
using std::endl;
using std::string;


/* -- Types --------------------------------------------------------------- */



template <typename V, typename E>
class Observer
{
public:
   virtual void next(V const & value) = 0;
   virtual void error(E const & err) = 0;
   virtual void complete() = 0;
};


//this is an intermediate observer, that is used in combination with the map functionality!!!
template <typename V, typename E>
class MappingObserver : public Observer<V,E>
{
public:
   Observer<V,E> * observer; //the actuall observer that wants to get notified
};



class Subscription
{
public:
   virtual void unsubscribe() = 0;

};



template <typename V, typename E>
class Observable : private Subscription //in this exampel, the Observable also implements the Subscription object...
{
private:
   //typedef for a C++ pointer to a member function
   //(that takes an pointer to an observer and returns an pointer to a subscription object)
   typedef Subscription * (Observable::*SubscribeHandler)(Observer<V,E> * observer);


   SubscribeHandler subscribeHandler;
   MappingObserver<V,E> * mappingObserver;
   Observable * mappingObservable;
   V const * values;
   size_t valuesCount;
   E const * err;


private:
   //constructor is private - as it shall not be called directly.
   //instead, a factory function like "from", "of" or "throwError" shall be used!
   Observable()
   {
      this->subscribeHandler = nullptr;
      this->mappingObservable = nullptr;
      this->values = nullptr;
      this->valuesCount = 0;
      this->err = nullptr;
   }


   //this is the method that is called when someone subscribes to the observable that was constructed...
   //...using the "of" method
   Subscription * subscribeHandler_of(Observer<V,E> * observer)
   {
      //call (the one and only) "next"
      observer->next(*values);
      //finally complete
      observer->complete();
      //prevent further invocation, by setting the handler fuction to NULL (as the observable has completed now!)
      subscribeHandler = nullptr;
      //as Observable derives from Subscription it is very easy at this point to return an Subscription object
      return this;
   }


   //this is the method that is called when someone subscribes to the observable that was constructed...
   //...using the "from" method
   Subscription * subscribeHandler_from(Observer<V,E> * observer)
   {
      //call one "next" after the other
      for (size_t i = 0; i < valuesCount; i++) observer->next(values[i]);
      //finally complete
      observer->complete();
      //prevent further invocation, by setting the handler fuction to NULL (as the observable has completed now!)
      subscribeHandler = nullptr;
      //as Observable derives from Subscription it is very easy at this point to return an Subscription object
      return this;
   }


   //this is the method that is called when someone subscribes to the observable that was constructed...
   //...using the "throwError" method
   Subscription * subscribeHandler_throwError(Observer<V,E> * observer)
   {
      //call error
      observer->error(*err);
      //finally complete
      observer->complete();
      //prevent further invocation, by setting the handler fuction to NULL (as the observable has completed now!)
      subscribeHandler = nullptr;
      //as Observable derives from Subscription it is very easy at this point to return an Subscription object
      return this;
   }


   //this is the method that is called when someone subscribes to the observable that was constructed...
   //... using the "map" method of another observable
   Subscription * subscribeHandler_map(Observer<V,E> * observer)
   {
      mappingObserver->observer = observer;
      return mappingObservable->subscribe(*mappingObserver);
      //TBD return subscription object of the mapping-observable or of "this"???
   }



   //this method implements Subscription::unsubscribe
   void unsubscribe()
   {
      //TBD!?
      //clear all
      this->values = nullptr;
      this->valuesCount = 0;
      this->err = nullptr;
   }


public:

   //factory function to construct a observable that emits a single value
   static Observable * of(V const & value) //value is reference to a const V
   {
      Observable * thiz = new Observable();
      thiz->subscribeHandler = &Observable::subscribeHandler_of;
      thiz->values = &value;
      // thiz->valuesCount = 1;
      return thiz;
   }

   //factory function to construct a observable that emits a series of values
   static Observable * from(V const * values, size_t count) //values is pointer to (array of) const V(s)
   {
      Observable * thiz = new Observable();
      thiz->subscribeHandler = &Observable::subscribeHandler_from;
      thiz->values = values;
      thiz->valuesCount = count;
      return thiz;
   }

   //factory function to construct a observable that emits an error
   static Observable * throwError(E const & err)
   {
      Observable * thiz = new Observable();
      thiz->subscribeHandler = &Observable::subscribeHandler_throwError;
      thiz->err = &err;
      return thiz;
   }


   //create a new Observable, that emits the "next-values" of this stream transformed by the given transformation function
   Observable * map(MappingObserver<V,E> & mappingObserver)
   {
      Observable * newobs = new Observable();
      newobs->subscribeHandler = &Observable::subscribeHandler_map;
      newobs->mappingObserver = &mappingObserver;
      newobs->mappingObservable = this;
      return newobs;
   }


   //this method is a wrapper to call the respective subscribe handler method, set at construction
   Subscription * subscribe(Observer<V,E> & observer)
   {
      //if the observable hasn't completed yet...
      if (this->subscribeHandler != nullptr)
      {
         //use c++ function-/method-pointer to...
         return (this->*subscribeHandler)(&observer); //...call either subscribeHandler_of/.._from/.._throwError
      }
      //otherwise: the observable has already completed
      //subscription has no further effect, than just returning an Subscription object
      return this; //as Observable derives from Subscription it is very easy at this point to return an Subscription object
   }

};




/* -- (Module) Global Variables ------------------------------------------- */


/* -- Implementation ------------------------------------------------------ */



//demo of an "Integer-Observer" (that takes integers and notifies an character-string in case of error)
class IntObserver : public Observer<int, char const *>
{
private:
   string id;

public:
   IntObserver(string id)
   {
      this->id = id;
   }

   void next(int const & value)
   {
      cout << id << ": " << value << endl;
   }

   void error(char const * const & err)
   {
      cout << id << ": " << err << endl;
   }

   void complete()
   {
      cout << id << ": complete!" << endl;
   }
};



//demo of an observer, that maps values and forwards everything the to "actual subscriber"
class IntMapObserver : public MappingObserver<int, char const *>
{
public:
   IntMapObserver()
   {
      forward = true;
   }

   void next(int const & value)
   {
      if (forward) //forward only every seconde value!!!
      {
         this->observer->next(2 * value); //double value and forward the the subscriber
      }
      forward = !forward; //toggle
   }

   void error(char const * const & err)
   {
      this->observer->error(err); //forward (unmodifed) error to subscriber
   }

   void complete()
   {
      this->observer->complete(); //forward complete to subscriber
   }

private:
   bool forward;
};




//typedef of an "Integer-Observerable" (that takes integers and notifies an character-string in case of error)
typedef Observable<int, const char *> IntObservable;


int main(int argc, char * argv[])
{
   IntObserver myIntObserver("IntObs");
   Subscription * mySubscription;

   cout << "--------------- TEST CASE 'of' ---------------" << endl;
   cout << "Creating a Single-Integer-Observable, that emits a single value before it completes." << endl;
   IntObservable * singleIntObservable = IntObservable::of(1);

   cout << "Now I am going to subscribe to the Single-Integer-Observable." << endl;
   mySubscription = singleIntObservable->subscribe(myIntObserver);

   cout << "OK, just for testing: I am going to subscribe to the Single-Integer-Observable, a second time..." << endl;
   cout << " But normally nothing should happen any more, as the observable should already be completed!" << endl;
   mySubscription = singleIntObservable->subscribe(myIntObserver);

   cout << "Now I am going to unsubscribe from the Single-Integer-Observable." << endl;
   mySubscription->unsubscribe();
   cout << endl;



   cout << "--------------- TEST CASE 'from' ---------------" << endl;
   cout << "Creating a Integer-Series-Observable, that emits a series of integer values before it completes." << endl;
   int series[] = { 1, -2, 3, -4, 5, -6, 7 };
   IntObservable * intSeriesObservable = IntObservable::from(series, 7);

   cout << "Now I am going to subscribe to the Integer-Series-Observable." << endl;
   mySubscription = intSeriesObservable->subscribe(myIntObserver);

   cout << "OK, just for testing: I am going to subscribe to the Integer-Series-Observable, a second time..." << endl;
   cout << " But normally nothing should happen any more, as the observable should already be completed!" << endl;
   mySubscription = intSeriesObservable->subscribe(myIntObserver);

   cout << "Now I am going to unsubscribe from that Integer-Series-Observable." << endl;
   mySubscription->unsubscribe();
   cout << endl;


   cout << "--------------- TEST CASE 'map' ---------------" << endl;
   cout << "Creating a Integer-Series-Observable, that emits a series of integer values before it completes." << endl;
   intSeriesObservable = IntObservable::from(series, 7);

   cout << "Map that Observable to another Observable by means of an inermediate mapping observer." << endl;
   cout << "The mapping observable forwards only every seconde value. The forwarded value will be doubled." << endl;
   IntMapObserver myMappingObserver;
   IntObservable * mappedSeriesObservable = intSeriesObservable->map(myMappingObserver);

   cout << "Now I am going to subscribe to the Mapped-Series-Observable." << endl;
   mySubscription = mappedSeriesObservable->subscribe(myIntObserver);

   cout << "OK, just for testing: I am going to subscribe to the Mapped-Series-Observable, a second time..." << endl;
   cout << " But normally nothing should happen any more, as the observable should already be completed!" << endl;
   mySubscription = mappedSeriesObservable->subscribe(myIntObserver);

   cout << "Now I am going to unsubscribe from that Integer-Series-Observable." << endl;
   mySubscription->unsubscribe();
   cout << endl;






   cout << "--------------- TEST CASE 'throwError' ---------------" << endl;
   cout << "Creating a Error-Observable, that emits an error text (c-string) before it completes." << endl;
   IntObservable * errorObservable = IntObservable::throwError("An error occured!");

   cout << "Now I am going to subscribe to the Error-Observable." << endl;
   mySubscription = errorObservable->subscribe(myIntObserver);

   cout << "OK, just for testing: I am going to subscribe to the Error-Observable, a second time..." << endl;
   cout << " But normally nothing should happen any more, as the observable should already be completed!" << endl;
   mySubscription = errorObservable->subscribe(myIntObserver);

   cout << "Now I am going to unsubscribe from that Error-Observable." << endl;
   mySubscription->unsubscribe();
   cout << endl;



   cout << endl << "---END---" << endl;
   return 0;
}

