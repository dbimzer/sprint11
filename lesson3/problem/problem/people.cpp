#include "people.h"

#include <stdexcept>

using namespace std;

Person::Person(const string& name, int age, Gender gender)
    :name_(name)
    ,age_(age)
    ,gender_(gender) {}

const std::string& Person::GetName() const
{
    return name_;
}

int Person::GetAge() const
{
    return age_;
}

Gender Person::GetGender() const
{
    return gender_;
}

Programmer::Programmer(const std::string& name, int age, Gender gender)
    :Person(name, age, gender) {}

void Programmer::AddProgrammingLanguage(ProgrammingLanguage language) {
    programming_languages_.insert(language);
}

bool Programmer::CanProgram(ProgrammingLanguage language) const {
    if (programming_languages_.count(language) > 0) {
        return true;
    }
    return false;
}

Worker::Worker(const std::string& name, int age, Gender gender)
    :Person(name, age, gender) {}

void Worker::AddSpeciality(WorkerSpeciality speciality) {
    specialities_.insert(speciality);
}

bool Worker::HasSpeciality(WorkerSpeciality speciality) const {
    if (specialities_.count(speciality) > 0) {
        return true;
    }
    return false;
}
