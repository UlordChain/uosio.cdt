// FIXME: %clang_cc1 -emit-llvm -triple %itanium_abi_triple -o - -std=c++11 %s | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -triple %itanium_abi_triple -o - -std=c++11 %s | FileCheck -check-prefix=CHECK-ASSIGN %s
// RUN: %clang_cc1 -emit-llvm -triple %itanium_abi_triple -o - -std=c++11 %s | FileCheck -check-prefix=CHECK-CTOR %s

// construct

struct E {
  E();
  E(E&&);
};

struct F {
  F();
  F(F&&);
};

struct G {
  E e;
};

struct H : G {
  F l;
  E m;
  F ar[2];
};

void f() {
  H s;
  // CHECK: call void @_ZN1HC1UOS_
  H t(static_cast<H&&>(s));
}


// assign

struct A {
  A &operator =(A&&);
};

struct B {
  B &operator =(B&&);
};

struct C {
  A a;
};

struct D : C {
  A a;
  B b;
  A ar[2];
};

void g() {
  D d;
  // CHECK: call {{.*}} @_ZN1DaSUOS_
  d = D();
}

// PR10822
struct I {
  unsigned var[1];
};

// CHECK: define void @_Z1hv() nounwind {
void h() {
  I i;
  // CHECK: call void @llvm.memcpy.
  i = I();
  // CHECK-NEXT: ret void
}

// PR10860
struct Empty { };
struct VirtualWithEmptyBase : Empty {
  virtual void f();
};

// CHECK: define void @_Z25move_VirtualWithEmptyBaseR20VirtualWithEmptyBaseS0_
void move_VirtualWithEmptyBase(VirtualWithEmptyBase &x, VirtualWithEmptyBase &y) {
  // CHECK: call {{.*}} @_ZN20VirtualWithEmptyBaseaSUOS_
  x = static_cast<VirtualWithEmptyBase&&>(y);
  // CHECK-NEXT: ret void
}

// move assignment ops

// CHECK-ASSIGN: define linkonce_odr {{.*}} @_ZN1DaSUOS_
// CHECK-ASSIGN: call {{.*}} @_ZN1CaSUOS_
// CHECK-ASSIGN: call {{.*}} @_ZN1AaSUOS_
// CHECK-ASSIGN: call {{.*}} @_ZN1BaSUOS_
// array loop
// CHECK-ASSIGN: br i1
// CHECK-ASSIGN: call {{.*}} @_ZN1AaSUOS_

// VirtualWithEmptyBase move assignment operatpr
// CHECK-ASSIGN: define linkonce_odr {{.*}} @_ZN20VirtualWithEmptyBaseaSUOS_
// CHECK-ASSIGN: store
// CHECK-ASSIGN-NEXT: store
// CHECK-ASSIGN-NOT: call
// CHECK-ASSIGN: ret

// CHECK-ASSIGN: define linkonce_odr {{.*}} @_ZN1CaSUOS_
// CHECK-ASSIGN: call {{.*}} @_ZN1AaSUOS_

// move ctors

// CHECK-CTOR: define linkonce_odr {{.*}} @_ZN1HC2UOS_
// CHECK-CTOR: call {{.*}} @_ZN1GC2UOS_
// CHECK-CTOR: call {{.*}} @_ZN1FC1UOS_
// CHECK-CTOR: call {{.*}} @_ZN1EC1UOS_
// array loop
// CHECK-CTOR: call {{.*}} @_ZN1FC1UOS_
// CHECK-CTOR: br i1

// CHECK-CTOR: define linkonce_odr {{.*}} @_ZN1GC2UOS_
// CHECK-CTOR: call {{.*}} @_ZN1EC1UOS_
