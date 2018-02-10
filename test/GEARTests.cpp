#include <iostream>
#include <string>
#include <cstdlib>

#include "Application.hpp"
#include "Object.hpp"
#include "Metadata.hpp"

void testApplication()
{
    Application app;
    app.Run();
}

// struct testObject01 : public Object
// {
//     ObjectRef ref_to_object02;

//     testObject01() : Object("Object 01")
//     {}
// };

// struct testObject02 : public Object
// {
//     ObjectRef ref_to_object02;

//     testObject02() : Object("Object 02")
//     {}
// };

// void testObjectRef()
// {
//     testObject01 object01;
//     testObject02 object02;

//     object01.ref_to_object02 = ObjectRef(&object02, &object01);
//     object02.ref_to_object02 = ObjectRef(&object01, &object02);

//     object01.debugPrint();
//     object02.debugPrint();
// }

void testMetadatas()
{
    //PrintTemplatedInt<10>();
    //std::tuple<int, std::string, float> t(1,"e", 1.5);
    //Foo<decltype(t), decltype(std::get<std::tuple_size<decltype(t)>::value - 1>(std::forward<decltype(t)>(t))), std::tuple_size<decltype(t)>::value - 1 >::Apply([](const auto& tupleMember){ std::cout<<tupleMember<<std::endl; }, std::forward<decltype(t)>(t));
};

int main()
{
    //testForeEachTuple();
    testMetaReflection();
    //testMetadatas();
    //testObjectRef();
    std::cin.get();
    testApplication();
}