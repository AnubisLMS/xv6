#ifndef XV6_MP_H
#define XV6_MP_H

// See MultiProcessor Specification Version 1.[14]

struct mp {                   // floating pointer
  unsigned char signature[4]; // "_MP_"
  void* physaddr;             // phys addr of MP config table
  unsigned char length;       // 1
  unsigned char specrev;      // [14]
  unsigned char checksum;     // all bytes must add up to 0
  unsigned char type;         // MP system config type
  unsigned char imcrp;
  unsigned char reserved[3];
};

struct mpconf {               // configuration table header
  unsigned char signature[4]; // "PCMP"
  unsigned short length;      // total table length
  unsigned char version;      // [14]
  unsigned char checksum;     // all bytes must add up to 0
  unsigned char product[20];  // product id
  unsigned int* oemtable;     // OEM table pointer
  unsigned short oemlength;   // OEM table length
  unsigned short entry;       // entry count
  unsigned int* lapicaddr;    // address of local APIC
  unsigned short xlength;     // extended table length
  unsigned char xchecksum;    // extended table checksum
  unsigned char reserved;
};

struct mpproc {               // processor table entry
  unsigned char type;         // entry type (0)
  unsigned char apicid;       // local APIC id
  unsigned char version;      // local APIC verison
  unsigned char flags;        // CPU flags
#define MPBOOT 0x02           // This proc is the bootstrap processor.
  unsigned char signature[4]; // CPU signature
  unsigned int feature;       // feature flags from CPUID instruction
  unsigned char reserved[8];
};

struct mpioapic {        // I/O APIC table entry
  unsigned char type;    // entry type (2)
  unsigned char apicno;  // I/O APIC id
  unsigned char version; // I/O APIC version
  unsigned char flags;   // I/O APIC flags
  unsigned int* addr;    // I/O APIC address
};

// Table entry types
#define MPPROC 0x00   // One per processor
#define MPBUS 0x01    // One per bus
#define MPIOAPIC 0x02 // One per I/O APIC
#define MPIOINTR 0x03 // One per bus interrupt source
#define MPLINTR 0x04  // One per system interrupt source

// PAGEBREAK!
// Blank page.

#endif // XV6_MP_H
