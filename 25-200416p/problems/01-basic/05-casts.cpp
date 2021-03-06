#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

struct Point2D { double x, y; };
struct Point3D : Point2D { double z; };

void foo(Point3D *p3) {
    p3->z = 10;
}

void legacy_foo(char *s) {
    assert(s[0] == 'h');
    assert(s[1] == 'i');
    assert(s[2] == 0);
}

struct some_opaque_user_data;
some_opaque_user_data* c_api_call(some_opaque_user_data *data) {
    char *ptr = (char*)data;
    // Do some magic with the pointer, but do not touch the data inside.
    ptr += 10;
    ptr -= 10;
    return (some_opaque_user_data*)ptr;
}

#pragma GCC diagnostic error "-Wwrite-strings"
#pragma GCC diagnostic error "-Wold-style-cast"

int main() {
    // START SOLUTION
    {
        Point3D *p3 = (Point3D*)std::malloc(sizeof(Point3D));
        new (p3) Point3D();  // Required even for simple types.
        Point2D *p2 = (Point2D*)p3;
        p2->x = (int)((2 + 3) / 2);
        p2->y = (int)20;
        foo((Point3D*)p3);
        p3->~Point3D();
        std::free(p3);
    }
    {
        Point3D *p3 = (Point3D*)new char[sizeof(Point3D)];
        new (p3) Point3D();  // Required even for simple types.
        Point2D *p2 = p3;
        p2->x = (int)((2 + 3) / 2);
        p2->y = (int)20;
        foo((Point3D*)p3);
        p3->~Point3D();
        delete[] (char*)p3;
    }
    {
        char s[] = "hi";
        legacy_foo(s);
    }
    {        
        legacy_foo("hi");
    }
    {        
        char *s = "hi";
        legacy_foo(s);
    }
    {
        const char *s = "hi";
        legacy_foo(s);
    }
    {
        using veci = std::vector<int>;
        const veci vec{1, 2, 3};
        const veci *pvec = c_api_call((some_opaque_user_data*)&vec);
        assert(pvec->size() == 3);
        assert(pvec->at(0) == 1);
        assert(pvec->at(1) == 2);
        assert(pvec->at(2) == 3);
        assert(vec == *pvec);
    }
    // END SOLUTION
    std::cout << "PASSED\n";
}
