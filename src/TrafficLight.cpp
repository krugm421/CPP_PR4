#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> myLock(this->_mutex);
    this->_cond.wait(myLock, [this](){
        return !(this->_deque.empty());
    });

    T element = std::move(this->_deque.back());
    this->_deque.pop_back();

    return element;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> myLock(this->_mutex);
    this->_deque.push_back(std::move(msg));
    this->_cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    this->_currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        auto current_traffic_light_phase = this->_message_queue.receive();
        if (current_traffic_light_pahse == TrafficLightPhase::green) return;   
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    this->threads.emplace_back(std::thread(
        TrafficLight::cycleThroughPhases, this
    ));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto T1 =
        std::chrono::steady_clock::now();

    auto rdn_cycle_time = [](){
        return static_cast<float>(rand())/RAND_MAX * (6000.0 - 4000.0) + 4000.0;
    };
    double period_time = rdn_cycle_time(); 

    while (true)
    {
        auto T2 =
            std::chrono::steady_clock::now();

        auto delta_T = std::chrono::duration_cast<std::chrono::milliseconds> (T2 - T1).count();
        if (delta_T >= period_time)
        {
            this->_currentPhase = 
                this->_currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            T1 = T2;
            period_time = rdn_cycle_time(); 

            // Send update
            auto p = this->_currentPhase;
            this->_message_queue.send(std::move(p));
        }        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}