# A *RxJS like* observable implementation in C++ (for training purposes)

This pice of code was inspired by the *Federico Knüssel's* great articel explaining the
the functionality of RxJS's observables.
See: https://medium.com/@fknussel/a-simple-observable-implementation-c9c809c89c69

As in its article a lot of nested lambda functions with closure expressions are used, which make it
complicate (for me - as a C/C++ programmer) to read and understand the functionality.
So for that reason, it tried to translate the implementation into C++.

The aim of this "project" is the same as *Federico Knüssel's* articel was intended to:
It shall help to understand what's going on under the hood when we work with RxJS!
In particular it is intendet to those readers with a C/C++ background.


As Federico wrote:
- An observer is just an object with three methods: next which takes in a value,
  error which takes in an error message and complete with has no arguments.

- An observable is just a function. This function takes in an observer as an argument,
  and returns a subscription object.

- A subscription object represents a disposable resource, such as the execution of an Observable.
  In RxJS a subscription object has a bunch of methods such as add and remove, but the most
  important one is *unsubscribe* which takes no argument and just disposes the resource held by the subscription.


In context of this "project":
- An *observer* is an abstract class, that defines the interface. User has to derive from that clas
  and implement there respective functionality/behaviour. (See the *IntObserver* in the example.)

- The *observable* is a templated class. It is instanciated using one of the *factory functions*
  `of`, `from` or `throwError`. The function used for instanciation defines the behaviour of the observable.
  It takes the values, that shall be emitted to the observer - on subscription.
  The *observable* starts it's "job" upon subscription of an observer.
  Method `map` can be used to create a new observable (by means of a intermediate mapping observable) that
  gets "mapped" values emmitted.

- The *subscription object* is returned on subcription to the *observable*. It is used to unsubscribe from
  the observable. In this implementation the *observable* IS the *subscription object* and it does nothing!
  So this may be not taken for "truth". Feel free to leave a comment about that ...



## How to build
Just compile it wich `g++ main.cpp`.


## Output
When executed, the program outputs the following:

```
--------------- TEST CASE 'of' ---------------
Creating a Single-Integer-Observable, that emits a single value before it completes.
Now I am going to subscribe to the Single-Integer-Observable.
IntObs: 1
IntObs: complete!
OK, just for testing: I am going to subscribe to the Single-Integer-Observable, a second time...
 But normally nothing should happen any more, as the observable should already be completed!
Now I am going to unsubscribe from the Single-Integer-Observable.


--------------- TEST CASE 'from' ---------------
Creating a Integer-Series-Observable, that emits a series of integer values before it completes.
Now I am going to subscribe to the Integer-Series-Observable.
IntObs: 1
IntObs: -2
IntObs: 3
IntObs: -4
IntObs: 5
IntObs: -6
IntObs: 7
IntObs: complete!
OK, just for testing: I am going to subscribe to the Integer-Series-Observable, a second time...
 But normally nothing should happen any more, as the observable should already be completed!
Now I am going to unsubscribe from that Integer-Series-Observable.


--------------- TEST CASE 'map' ---------------
Creating a Integer-Series-Observable, that emits a series of integer values before it completes.
Map that Observable to another Observable by means of an inermediate mapping observer.
The mapping observable forwards only every seconde value. The forwarded value will be doubled.
Now I am going to subscribe to the Mapped-Series-Observable.
IntObs: 2
IntObs: 6
IntObs: 10
IntObs: 14
IntObs: complete!
OK, just for testing: I am going to subscribe to the Mapped-Series-Observable, a second time...
 But normally nothing should happen any more, as the observable should already be completed!
Now I am going to unsubscribe from that Integer-Series-Observable.


--------------- TEST CASE 'throwError' ---------------
Creating a Error-Observable, that emits an error text (c-string) before it completes.
Now I am going to subscribe to the Error-Observable.
IntObs: An error occured!
IntObs: complete!
OK, just for testing: I am going to subscribe to the Error-Observable, a second time...
 But normally nothing should happen any more, as the observable should already be completed!
Now I am going to unsubscribe from that Error-Observable.
```

## Additional
Additional you can see in this project how to use C++ function to method pointers.
This is always a bit tricky (at least for me) how such function pointers a declared, assigned
and invocated.

A declaration (by help of a typedef) is like that:
```
typedef Subscription * (Observable::*SubscribeHandler)(Observer<V,E> * observer);
SubscribeHandler subscribeHandler;
```

Assignment requires the right hand side to be "full qualifiied".
That means the preceding socpe (Observable::) is mandatory:
```
thiz->subscribeHandler = &Observable::subscribeHanlder_from;
```

Invocation requires the function pointer to be wrapped in parentheses and using the `->*` operator
together with the instance object (in this case `this`):
```
(this->*subscribeHandler)(&observer);
```


