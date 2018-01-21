GreyBus
=======

Introduction (Informative)
==========================

The Greybus Specification describes a suite of communications protocols
required to support the Project Ara modular cell phone platform.

The Project Ara Module Developer's Kit (MDK) is the official Project Ara
platform definition; it comprises various documents which collectively
define the Ara platform, including its industrial, mechanical,
electrical, and software design and requirements. Refer to the main MDK
document for an introduction to the platform and its components.
Familiarity with this document is assumed throughout the Greybus
Specification; its definitions are incorporated here by reference.

The Greybus Specification is included within the MDK; its purpose is to
define software interfaces whose data and control flow cross Module
boundaries. This is required to ensure software compatibility and
interoperability between Modules and the Endo.

Project Ara utilizes the *unipro* protocol for inter-Module
communication. The *unipro* specification is defined by the |mipi|
Alliance. *unipro*'s design follows a layered architecture, and
specifies how communication shall occur up to the Application layer in
the [OSI
model](http://www.ecma-international.org/activities/Communications/TG11/s020269e.pdf).
Project Ara's architecture requires an application layer specification
which can handle dynamic device insertion and removal from the system at
any time and at variable locations. It also requires that existing
Modules interoperate with Modules introduced at later dates. This
document aims to define a suite of application layer protocols which
meet these needs.

In addition to *unipro*, Project Ara also specifies a small number of
other interfaces between Modules and the Endo. These include a power
bus, signals which enable hotplug and power management functions, and
interface pins for Modules which emit and receive radio signals. The
Greybus Specification also defines the behavior of the system's software
with respect to these interfaces.

A Project Ara "Module" is a device that slides into a physical slot on a
Project Ara Endo. A module has one or more "Interface Blocks." Each
Interface Block is a single physical port through which *unipro* packets
are transferred. Greybus represents each Interface Block with an
"Interface" abstraction. A Greybus Interface can support one or more
"Bundles". A Bundle represents a logical "device" in Greybus that does
one logical "thing" as far as the host operating system works. Bundles
communicate with each other on the network via one or more *unipro*
CPorts. A CPort is a bidirectional pipe through which *unipro* traffic
is exchanged. Bundles send "messages" via CPorts; messages are datagrams
with ancillary metadata. All CPort traffic is peer-to-peer; multicast
communication is not supported.

Project Ara presently requires that exactly one application processor
(AP) is present on the system for storing user data and executing
applications. The Module that contains the AP is the *AP Module*; the
Greybus specification defines a control-protocol to allow the AP Module
to accomplish its tasks.

In order to ensure interoperability between the wide array of
application processors and hardware peripherals commonly available on
mobile handsets, the Greybus Specification defines a suite of
device-class-protocols, which allow for communication between the
various Modules on the system, regardless of the particulars of the
chipsets involved.

The main functional chipsets on Modules may communicate via a native
*unipro* interface or via "bridges," special-purpose ASICs which
intermediate between these chipsets and the *unipro* network. In order
to provide a transition path for chipsets without native *unipro*
interfaces, the Greybus Specification defines a variety of
bridged-phy-protocols, which allow Module developers to expose these
existing protocols to the network. In addition to providing an "on-ramp"
to the platform, this also allows the implementation of Modules which
require communication that does not comply with a device class Protocol.

Greybus Hardware Model
======================

An implementation of the Project Ara platform which complies with the
Greybus Specification is a *Greybus system*.

A Greybus system shall be composed of the following physical components:

1.  An "endoskeleton" (or *Endo*), consisting of the following elements:
    -   One or more *unipro* switches, which distribute *unipro* network
        traffic throughout the Greybus network.
    -   One or more *interface blocks*. These are the connectors which
        expose the Endo's communication interface to other elements in a
        Greybus system.
    -   Exactly one Supervisory Controller, hereafter referred to as the
        "SVC." The SVC administers the Greybus system, including the
        system's *unipro* switches, its power bus, its wake/detect pins,
        and its RF bus.

2.  One or more Modules, which are physically inserted into slots on the
    Endo. Modules shall implement Communication Protocols in accordance
    with this document's specifications.
3.  Exactly one Application Processor Module, hereafter referred to as
    the "AP Module."

For a full description of the Project Ara platform, please see the
*Project Ara Module Developers Kit* specification.

Interface Information
=====================

A Greybus Interface shall provide self-descriptive information in order
to establish communications with other Interfaces on the *unipro*
network. This information is provided via a Manifest, which describes
components present within the Interface that are accessible via
*unipro*. The Manifest is a data structure, which includes a set of
Descriptors, that presents a functional description of the Interface.
Together, these Descriptors define the Interface's capabilities and
means of communication via *unipro* from the perspective of the
application layer and above.

Data Requirements
-----------------

All data found in Manifest structures defined below shall adhere to the
following general requirements:

-   All numeric values shall be unsigned unless explicitly stated
    otherwise.
-   All descriptor field values shall have little endian format.
-   Numeric values prefixed with 0x are hexadecimal; they are decimal
    otherwise.
-   All offset and size values are expressed in units of bytes unless
    explicitly stated otherwise.
-   All string descriptors shall consist of UTF-8 encoded characters.
-   All headers and descriptor data within a Manifest shall be
    implicitly followed by pad bytes as necessary to bring the
    structure's total size to a multiple of 4 bytes.
-   Accordingly, the low-order two bits of all header *size* field
    values shall be 00.
-   Any reserved or unused space (including implicit padding) in a
    header or descriptor shall be ignored when read, and zero-filled
    when written.
-   All major structures (like the Manifest header) and interface
    Protocols (like that between the AP Module and SVC) shall be
    versioned, to allow future extensions (or fixes) to be added and
    recognized.

Manifest
--------

The Manifest is a contiguous block of data that includes a Manifest
Header and a set of Descriptors. When read, a Manifest is transferred in
its entirety. This allows the Interface to be described to the AP Module
all at once, alleviating the need for multiple communication messages
during the enumeration phase of the Interface.

### Manifest Header

The Manifest Header is present at the beginning of the Manifest and
defines the size of the manifest and the version of the Greybus Protocol
with which the Manifest complies.

    :label: table-manifest-header

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        size            2       Number          Size of the entire manifest
    2        version_major   1       |gb-major|      Greybus major version
    3        version_minor   1       |gb-minor|      Greybus minor version
    =======  ==============  ======  ==========      ===========================

The values of version\_major and version\_minor values shall refer to
the highest version of this document (currently |gb-major|.
|gb-minor|) with which the format complies.

Minor versions increment with modifications to the Greybus definition,
in such a way that any Protocol handler that supports the version\_major
can correctly interpret a Manifest in the modified format. A changed
version\_major indicates major differences in the Manifest format. It is
not expected that a parser can properly interpret a Manifest whose
version\_major is greater than the version\_major supported by the
parser.

All Manifest parsers shall be able to interpret manifests formatted
using older (lower numbered) Greybus versions, such that they still work
properly (i.e. backwards compatibility is required).

The layout for the Manifest Header can be seen in Table
table-manifest-header.

### Descriptors

    :label: table-descriptor-header

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        size            2       Number          Size of this descriptor
    2        type            1       Number          :ref:`descriptor-type`
    3        (pad)           1       0               Reserved (pad to 4 bytes)
    =======  ==============  ======  ==========      ===========================


Following the Manifest Header is one or more Descriptors. Each
Descriptor is composed of a Descriptor Header followed by Descriptor
Data. The format of the Descriptor Header can be seen in Table
table-descriptor-header.

#### Descriptor type

The format of the Descriptor Data depends on the type of the descriptor,
which is specified in the header. The known descriptor types and their
values are described in Table table-descriptor-type.

    :label: table-descriptor-type

    ============================    ==========
    Descriptor Type                 Value
    ============================    ==========
    Invalid                         0x00
    Interface                       0x01
    String                          0x02
    Bundle                          0x03
    CPort                           0x04
    (All other values reserved)     0x05..0xff
    ============================    ==========


### Interface Descriptor

Interface descriptor describes an access point for a Module to the
*unipro* network. Each interface represents a single physical port
through which *unipro* packets are transferred. Every Module shall have
at least one interface. Each interface has an unique ID within the Endo.

This descriptor describes Interface-specific values as set by the vendor
who created the Interface. Every Manifest shall have exactly one
Interface descriptor as described in Table table-interface-descriptor.

    :label: table-interface-descriptor

    =======  =================  ======  ==========  ==============================
    Offset   Field              Size    Value       Description
    =======  =================  ======  ==========  ==============================
    0        size               2       0x0008      Size of this descriptor
    2        type               1       0x01        Type of the descriptor (Interface)
    3        (pad)              1       0           Reserved (pad to 4 byte boundary)
    4        vendor_string_id   1       ID          String ID for the vendor name
    5        product_string_id  1       ID          String ID for the product name
    6        (pad)              2       0           Reserved (pad to 4 byte boundlary)
    =======  =================  ======  ==========  ==============================

*vendor\_string\_id* is a reference to a specific string descriptor ID
that provides a description of the vendor who created the Module. If
there is no string present for this value in the Manifest, this value
shall be 0x00. See the string-descriptor section below for more details.

*product\_string\_id* is a reference to a specific string descriptor ID
that provides a description of the product. If there is no string
present for this value in the Manifest, this value shall be 0x00. See
the string-descriptor section below for more details.

### String Descriptor

A string descriptor provides a human-readable string for a specific
value, such as a vendor or product string. Strings consist of UTF-8
characters and are not required to be zero terminated. A string
descriptor shall be referenced only once within the Manifest, e.g. only
one product (or vendor) string field may refer to string ID 2. The
format of the string descriptor can be found in Table
table-string-descriptor.

    :label: table-string-descriptor

    ============  ==============  ========  ==========  ===========================
    Offset        Field           Size      Value       Description
    ============  ==============  ========  ==========  ===========================
    0             size            2         Number      Size of this descriptor
    2             type            1         0x02        Type of the descriptor (String)
    3             (pad)           1         0           Reserved (pad to 4 byte boundary)
    4             length          1         Number      Length of the string in bytes
    5             id              1         ID          String ID for this descriptor
    6             string          *length*  UTF-8       Characters for the string
    6+\ *length*  (pad)           0-3       0           Reserved (pad to 4 byte boundary)
    ============  ==============  ========  ==========  ===========================


The *id* field shall not be 0x00, as that is an invalid String ID value.

The *length* field excludes any trailing padding bytes in the
descriptor.

### Bundle Descriptor

A Bundle represents a device in Greybus. Bundles communicate with each
other on the network via one or more *unipro* CPorts.

    :label: table-bundle-descriptor

    ============  ==============  ========  ==========  ===========================
    Offset        Field           Size      Value       Description
    ============  ==============  ========  ==========  ===========================
    0             size            2         0x0008      Size of this descriptor
    2             type            1         0x03        Type of the descriptor (Bundle)
    3             (pad)           1         0           Reserved (pad to 4 byte boundary)
    4             id              1         ID          Interface-unique ID for this Bundle
    5             class           1         Number      See Table table-bundle-class
    6             (pad)           2         0           Reserved (pad to 8 bytes)
    ============  ==============  ========  ==========  ===========================


The *id* field uniquely identifies a Bundle within the Interface. The
first Bundle shall have ID 0, the second (if present) shall have value
1, and so on. The purpose of these Ids is to allow CPort descriptors to
define which Bundle they are associated with. The Bundle descriptor is
defined in Table table-bundle-descriptor.

The *class* field defines the class of the bundle. This shall be used by
the AP to find what to expect from the bundle and how to configure/use
it. Class types are defined in Table table-bundle-class.

    :label: table-bundle-class

    ============================    ==========
    Class type                      Value
    ============================    ==========
    Control                         0x00
    SVC                             0x01
    GPIO                            0x02
    I2C                             0x03
    UART                            0x04
    HID                             0x05
    USB                             0x06
    SDIO                            0x07
    Battery                         0x08
    PWM                             0x09
    I2S                             0x0a
    SPI                             0x0b
    Display                         0x0c
    Camera                          0x0d
    Sensor                          0x0e
    Lights                          0x0f
    Vibrator                        0x10
    (Reserved values)               0x11..0xee
    Power Transfer                  0xef
    Vendor Specific                 0xff
    ============================    ==========


### CPort Descriptor

A CPort descriptor describes a CPort implemented within the Module. Each
CPort is associated with one of the Interface's Bundle, and has an ID
unique for that Interface. Every CPort defines the Protocol used by the
AP Module to interact with the CPort. A special control CPort shall be
defined for every Interface, and shall be defined to use the *Control
Protocol*. The Cport Descriptor is defined in Table
table-cport-descriptor. The details of these Protocols are defined in
the sections device-class-protocols and bridged-phy-protocols below.

    :label: table-cport-descriptor

    ========  ==============  ======  ==========  ===========================
    Offset    Field           Size    Value       Description
    ========  ==============  ======  ==========  ===========================
    0         size            2       0x0008      Size of this descriptor
    2         type            1       0x04        Type of the descriptor (CPort)
    3         (pad)           1       0           Reserved (pad to 4 byte boundary)
    4         id              2       ID          ID (destination address) of the CPort
    6         bundle          1       ID          Bundle ID this CPort is associated with
    7         protocol        1       Number      See Table table-cport-protocol
    ========  ==============  ======  ==========  ===========================


The *id* field is the CPort identifier used by other Modules to direct
traffic to this CPort. The IDs for CPorts using the same Interface shall
be unique. Certain low-numbered CPort identifiers (such as the control
CPort) are reserved. Implementors shall assign CPorts low-numbered ID
values, generally no higher than 31. (Higher-numbered CPort ids impact
on the total usable number of *unipro* devices and typically should not
be used.)

    :label: table-cport-protocol

    ============================    ==========
    Protocol                        Value
    ============================    ==========
    Control                         0x00
    AP                              0x01
    GPIO                            0x02
    I2C                             0x03
    UART                            0x04
    HID                             0x05
    USB                             0x06
    SDIO                            0x07
    Battery                         0x08
    PWM                             0x09
    I2S Management                  0x0a
    SPI                             0x0b
    Display                         0x0c
    Camera                          0x0d
    Sensor                          0x0e
    Lights                          0x0f
    Vibrator                        0x10
    Loopback                        0x11
    I2S Receiver                    0x12
    I2S Transmitter                 0x13
    SVC                             0x14
    Firmware                        0x15
    (All other values reserved)     0x16..0xfd
    Raw                             0xfe
    Vendor Specific                 0xff
    ============================    ==========

Greybus Operations
==================

Greybus communication is built on the use of *unipro* messages to send
information between Modules. And although *unipro* offers reliable
transfer of data frames between interfaces, it is often necessary for
the sender to know whether the effects of sending a message were what
was expected. For example, a request sent to a *unipro* switch
controller requesting a reconfiguration of the routing table could fail,
and proceeding as if a failure had not occurred in this case leads to
undefined (and possibly dangerous) behavior. Similarly, the AP Module
likely needs to retrieve information from other Modules; this requires
that a message requesting information be paired with a returned message
containing the information requested.

For this reason, Greybus performs communication between Modules using
Greybus Operations. A Greybus Operation defines an activity (such as a
data transfer) initiated in one Module that is implemented (or executed)
by another. The particular activity performed is defined by the
operation's type. An operation is generally implemented by a pair of
messages--one containing a request, and the other containing a response,
but *unidirectional* operations (i.e. requests without matching
responses) are also supported. Both messages contain a simple header
that includes the type of the operation and size of the message. In
addition, each operation has a unique ID, and both messages in an
operation contain this value so a response can be associated with its
matching request (unidirectional operations use a reserved ID). Finally,
all responses contain a byte in message header to communicate status of
the operation--either success or a reason for a failure.

Whether a particular operation has a response message or not (i.e. is
unidirectional) is protocol dependent. It usually makes sense for
operations which may be initiated by the AP Module to have responses as
any errors can be logged and often also reported up the stack (e.g. to
userspace).

Operations are performed over Greybus Connections. A connection is a
communication path between two Modules. Each end of a connection is a
*unipro* CPort, associated with a particular interface in a Greybus
Module. A connection can be established once the AP Module learns of the
existence of a CPort in another Module. The AP Module shall allocate a
CPort for its end of the connection, and once the *unipro* network
switch is configured properly the connection can be used for data
transfer (and in particular, for operations).

Each CPort in a Greybus Module has associated with it a Protocol. The
Protocol dictates the way the CPort interprets incoming operation
messages. Stated another way, the meaning of the operation type found in
a request message depends on which Protocol the connection uses.
Operation type 5 might mean "receive data" in one Protocol, while
operation 5 might mean "go to sleep" in another. When the AP Module
establishes a connection with a CPort in another Module, that connection
uses the CPort's advertised Protocol.

Each Greybus Protocol has a two-byte version associated with it. This
allows Protocols to evolve, and provides a way for Protocol handling
software to correctly handle messages transferred over a connection. A
Protocol version, for example 0.1, consists of a major and minor part (0
and 1 in this case). The major version is changed (increased) if new
features of the Protocol are incompatible with existing Protocol
handling code. Protocol changes (such as the addition of optional
features) that do not require software changes are indicated by a change
to the minor version number.

Greybus Protocol handling code shall record the maximum Protocol version
it can support. It shall also support all versions (major and minor) of
the Protocol lower than that maximum. The Protocol handlers on the two
ends of the connection negotiate the version of the Protocol to use when
a connection is established. Every Protocol implements a *version*
operation for this purpose. The version request message contains the
major and minor Protocol version supported by the sending side. The
receiving end decides whether that version should be used, or if a
different (lower) version that it supports should be used instead. Both
sides use the version of the Protocol contained in the response.

The Greybus Operations mechanism forms a base layer on which other
Protocols are built. Protocols define the format of request messages,
their expected response data, and the effect of the request on state in
one or both Modules. Users of a Protocol can rely on the Greybus core
getting the operation request message to its intended target, and
transferring the operation status and any other data back. In the
explanations that follow, we refer to the interface through which a
request operation is sent as the source, and the interface from which
the response is sent as the destination.

Message Data Requirements
-------------------------

All data found in message structures defined below shall adhere to the
following general requirements:

-   All numeric values shall be unsigned unless explicitly stated
    otherwise.
-   All numeric field values shall have little endian format.
-   Numeric values prefixed with 0x are hexadecimal; they are decimal
    otherwise.
-   All offset and size values are expressed in units of bytes unless
    explicitly stated otherwise.
-   All string values shall consist of UTF-8 encoded characters.
-   String values shall be paired with a numeric value indicating the
    number of characters in the string.
-   String values shall not include terminating NUL characters.
-   Any reserved space in a message structure shall be ignored when
    read, and zero-filled when written.
-   All Protocols shall be versioned, to allow future extensions (or
    fixes) to be added and recognized.

Fields within a message payload have no specific alignment requirements.
Message headers are padded to fill 8 bytes, so the alignment of a
message's payload is comparable to that of its header. If alignment is
required, it is achieved using explicitly defined reserved fields.

Operation Messages
------------------

Operation request messages and operation response messages have the same
basic format. Each begins with a short header, and is followed by
payload data. A response message records an additional status value in
the header, and both requests and responses may have a zero-byte
payload.

### Operation Message Header

Table table-operation-message-header summarizes the format of an
operation message header.

    :label: table-operation-message-header

    ========  ==============  ======  ==========      ===========================
    Offset    Field           Size    Value           Description
    ========  ==============  ======  ==========      ===========================
    0         size            2       Number          Size of this operation message
    2         id              2       ID              Requestor-supplied unique request identifier
    4         type            1       Number          Type of Greybus operation (Protocol-specific)
    5         status          1       Number          Operation result (response message only)
    6         (pad)           2       0               Reserved (pad to 8 bytes)
    ========  ==============  ======  ==========      ===========================

The *size* includes the operation message header as well as any payload
that follows it. As mentioned earlier, the meaning of a type value
depends on the Protocol in use on the connection carrying the message.
Only 127 operations are available for a given Protocol, 0x01..0x7f.
Operation 0x00 is reserved as an invalid value for all Protocols. The
high bit (0x80) of an operation type is used as a flag that
distinguishes a request operation from its response. For requests, this
bit is 0, for responses, it is 1. For example the request and response
messages for operation 0x0a contain 0x0a and 0x8a (respectively) in
their type fields. The ID allows many operations to be "in flight" on a
connection at once. The special ID 0 is reserved for unidirectional
operations.

A connection Protocol is defined by describing the format of the
operations supported by the Protocol. Each operation specifies the
payload portions of the request and response messages used for the
Protocol, along with all actions or state changes that take place as a
result of the operation.

Connection Protocols
====================

The following sections define the request and response message formats
for all operations for specific connection Protocols. Requests are most
often (but not always) initiated by the AP Module. Each request has a
unique identifier, supplied by the requestor, and each response includes
the identifier of the request with which it is associated. This allows
operations to complete asynchronously, so multiple operations can be "in
flight" between the AP Module and a *unipro*-attached adapter at once.

Each response includes a status byte in its message header, which
communicates whether any error occurred in delivering or processing a
requested operation. If the operation completed successfully, the status
value is zero. Otherwise, the reason it was not successful is conveyed
by one of the positive values defined in Table
table-connection-status-values.

Protocol Status
---------------

A Protocol can define its own status values if needed. These status
values shall lie within the range defined by the "(Reserved for Protocol
use)" table entry in Table table-connection-status-values. Every status
byte with a MSB set to one other than 0xff is a valid Protocol status
value.

    :label: table-connection-status-values

    ============================  ===============  =======================
    Status                        Value            Meaning
    ============================  ===============  =======================
    GB_OP_SUCCESS                 0x00             Operation completed successfully
    GB_OP_INTERRUPTED             0x01             Operation processing was interrupted
    GB_OP_TIMEOUT                 0x02             Operation processing timed out
    GB_OP_NO_MEMORY               0x03             Memory exhaustion prevented operation completion
    GB_OP_PROTOCOL_BAD            0x04             Protocol is not supported by this Greybus implementation
    GB_OP_OVERFLOW                0x05             Request message was too large
    GB_OP_INVALID                 0x06             Invalid argument supplied
    GB_OP_RETRY                   0x07             Request should be retried
    GB_OP_NONEXISTENT             0x08             The device does not exist
    Reserved                      0x09 to 0x7f     Reserved for future use
    Reserved for Protocol use     0x80 to 0xfd     Status defined by the Protocol in use
    GB_OP_UNKNOWN_ERROR           0xfe             Unknown error occured
    GB_OP_INTERNAL                0xff             Invalid initial value.
    ============================  ===============  =======================

Values marked *Reserved for Protocol use* are to be used by the
individual Protocols as defined in the device-class-protocols and
bridged-phy-protocols sections below.

Note that *GB\_OP\_INTERNAL* should never be used in a response message.
It is reserved for internal use by the Greybus application stack only.

All Protocols defined herein are subject to the
message-data-requirements listed above.

Protocol Versions
-----------------

Every Protocol has a version, which comprises two one-byte values, major
and minor. A Protocol definition can evolve to add new capabilities, and
as it does so, its version changes. If existing (or old) Protocol
handling code which complies with this specification can function
properly with the new feature in place, only the minor version of the
Protocol shall change. Any time a Protocol changes in a way that
requires the handling code be updated to function properly, the
Protocol's major version shall change.

Two Modules may implement different versions of a Protocol, and as a
result they shall negotiate a common version of the Protocol to use.
This is done by each side exchanging information about the version of
the Protocol it supports at the time a connection between Module
interfaces is set up. The version of a particular Protocol advertised by
a Module is the same as the version of the document that defines the
Protocol (so for Protocols defined herein, the version is |gb-major|.
|gb-minor|). In the future, if the Protocol specifications are removed
from this document, the versions will become independent of the overall
Greybus Specification document.

To agree on a Protocol, an operation request supplies the (greatest)
major and minor version of the Protocol supported by the source of a
request. The request destination compares that version with the
(greatest) version of the Protocol it supports. The version that is the
largest common version number of the Protocol sent by both sides shall
be the version that is to be used in communication between the devices.
This chosen version is returned back as a response of the request. As a
consequence of this, Protocol handlers shall be capable of handling all
prior versions of the Protocol.

Special Protocols
=================

This section defines three Protocols, each of which serves a special
purpose in a Greybus system.

The first is the Control Protocol. Every Interface shall provide a CPort
that uses the Control Protocol. It is used by the AP Module to notify
Interfaces when connections are available for them to use.

The second is the SVC Protocol, which is used only between the SVC and
AP Module. The SVC provides low-level control of the *unipro* network.
The SVC performs almost all of its activities under direction of the AP
Module, and the SVC Protocol is used by the AP Module to exert this
control. The SVC also uses this protocol to notify the AP Module of
events, such as the insertion or removal of a Module.

The third is the Firmware Protocol, which is used between the AP Module
and any other module's bootloader to download firmware executables to
the module. When a module's manifest includes a CPort using the Firmware
Protocol, the AP can connect to that CPort and download a firmware
executable to the module.

Control Protocol
----------------

All Interfaces are required to define a CPort that uses the Control
Protocol, and shall be prepared to receive Operation requests on that
CPort at any time. The CPort that uses the Control Protocol must have an
id of '0'. CPort id '0' is a reserved CPort address for the Control
Protocol. Similarly the bundle descriptor associated with the Control
CPort must have an id of '0'. Bundle id '0' is a reserved id for the
Control Protocol bundle descriptor.

A Greybus connection is established whenever a control connection is
used, but the Interface is never notified that such a connection exists.
Only the AP Module is able to send control requests. Any other Interface
shall only send control response messages, and such messages shall only
be sent in reply to a request received on its control CPort.

Conceptually, the Operations in the Greybus Control Protocol are:

    int version(u8 offer_major, u8 offer_minor, u8 *major, u8 *minor);

Negotiates the major and minor version of the Protocol used for
communication over the connection.  The AP offers the
version of the Protocol it supports.  The Interface replies with
the version that will be used--either the one offered if
supported or its own (lower) version otherwise.  Protocol
handling code adhering to the Protocol specified herein supports
major version |gb-major|, minor version |gb-minor|.

    int get_manifest_size(u16 *size);

This Operation is used by the AP to discover the size of a module's
Interface Manifest.  This is used after the SVC has discovered which
Module contains the AP.  The response to this Operation contains the
size of the manifest, which is used by the AP to fetch the manifest
later.  This operation is only initiated by the AP.

     int get_manifest(u8 *manifest);

This Operation is used by the AP after the SVC has discovered
which Module contains the AP.  The response to this Operation
contains the manifest of the Module, which is used by the AP to
determine the functionality module provides.  This operation is only
initiated by the AP.

    int connected(u16 cport_id);

This Operation is used to notify an Interface that a Greybus
connection has been established using the indicated CPort.
Upon receiving this request, an Interface shall be prepared to
receive messages on the indicated CPort.  The Interface may send
messages over the indicated CPort once it has sent a response
to the connected request.  This operation is never used for
control CPort.

    int disconnected(u16 cport_id);

This Operation is used to notify an Interface that a previously
established Greybus connection may no longer be used.  This
operation is never used for control CPort.

    int get_ids(u32 unipro_mfg_id, unipro_prod_id,
                            ara_vend_id, ara_prod_id, u64 uuid_low,
                            u64 uuid_high);

Mods specific operation to retrieve the identifying information
for the mods.

### Greybus Control Operations

All control Operations are contained within a Greybus control request
message. Every control request results in a matching response. The
request and response messages for each control Operation are defined
below.

Table table-control-operation-type defines the Greybus Control Protocol
Operation types and their values. Both the request type and response
type values are shown.

    :label: table-control-operation-type

    ===========================  =============  ==============
    Control Operation Type       Request Value  Response Value
    ===========================  =============  ==============
    Invalid                      0x00           0x80
    Protocol Version             0x01           0x81
    Reserved                     0x02           0x82
    Get Manifest Size            0x03           0x83
    Get Manifest                 0x04           0x84
    Connected                    0x05           0x85
    Disconnected                 0x06           0x86
    Get IDs                      0x7f           0xff
    (all other values reserved)  0x07..0x7f     0x87..0xff
    ===========================  =============  ==============

### Greybus Control Protocol Version Operation

The Greybus Control Protocol version operation allows the Protocol
handling software on both ends of a connection to negotiate the version
of the Control Protocol to use.

#### Greybus Control Protocol Version Request

Table table-control-version-request defines the Greybus Control version
request payload. The request supplies the greatest major and minor
version of the Control Protocol supported by the sender.

    :label: table-control-version-request

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      Offered Control Protocol major version
    1        version_minor   1       |gb-minor|      Offered Control Protocol minor version
    =======  ==============  ======  ==========      ===========================

#### Greybus Control Protocol Version Response

The Greybus Control Protocol version response payload contains two
one-byte values, as defined in table
table-control-protocol-version-response. A Greybus Control controller
adhering to the Protocol specified herein shall report major version
|gb-major|, minor version |gb-minor|.

    :label: table-control-protocol-version-response

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      Control Protocol major version
    1        version_minor   1       |gb-minor|      Control Protocol minor version
    =======  ==============  ======  ==========      ===========================

### Greybus Control Get Manifest Size Operation

The Greybus control get manifest size Operation is used by the AP for
all non-AP Interfaces (other than interface zero, which belongs to the
SVC), on hotplug event, to determine the size of the manifest.

#### Greybus Control Get Manifest Size Request

The Greybus control get manifest size request is sent by the AP to all
non-AP modules. The Greybus control get manifest size request message
has no payload.

#### Greybus Control Get Manifest Size Response

The Greybus control get manifest size response contains a two byte field
'size'.

    :label: table-control-get-manifest-size-response

    =======  ==============  ===========  ==========      ===========================
    Offset   Field           Size         Value           Description
    =======  ==============  ===========  ==========      ===========================
    0        size            2            Number          Size of the Manifest
    =======  ==============  ===========  ==========      ===========================

### Greybus Control Get Manifest Operation

The Greybus control get manifest Operation is used by the AP for all
non-AP Interfaces (other than interface zero, which belongs to the SVC),
on hotplug event, to determine the functionality provided by the module
via that interface.

#### Greybus Control Get Manifest Request

The Greybus control get manifest request is sent by the AP to all non-AP
modules. The Greybus control get manifest request message has no
payload.

#### Greybus Control Get Manifest Response

The Greybus control get manifest response contains a block of data, that
describes the functionality provided by the module. This block of data
is also known as manifest-description.

    :label: table-control-get-manifest-response

    =======  ==============  ===========  ==========      ===========================
    Offset   Field           Size         Value           Description
    =======  ==============  ===========  ==========      ===========================
    0        manifest        *size*       Data            Manifest
    =======  ==============  ===========  ==========      ===========================

### Greybus Control Connected Operation

The Greybus Control Connected Operation is sent to notify an Interface
that one of its CPorts (other than control CPort) now has a connection
established. The SVC sends this request when it has set up a Greybus SVC
connection with an AP Module Interface. The AP Module sends this request
to other Interfaces when it has set up Greybus connections for them to
use.

#### Greybus Control Connected Request

The Greybus control connected request supplies the CPort ID on the
receiving Interface that has been connected.

    :label: table-control-connected-request

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
    0        cport_id        2       CPort ID        CPort that is now connected
    =======  ==============  ======  ============    ===========================

#### Greybus Control Connected Response

The Greybus control connected response message contains no payload.

### Greybus Control Disconnected Operation

The Greybus control disconnected Operation is sent to notify an
Interface that a CPort (other than control CPort) that was formerly the
subject of a Greybus Control Connected Operation shall no longer be
used. No more messages may be sent over this connection, and any
messages received shall be discarded.

#### Greybus Control Disconnected Request

The Greybus control disconnected request supplies the CPort ID on the
receiving Interface that is no longer connected.

    :label: table-control-disconnected-request

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
    0        cport_id        2       CPort ID        CPort that is now disconnected
    =======  ==============  ======  ============    ===========================

#### Greybus Control Disconnected Response

The Greybus control disconnected response message contains no payload.

### Greybus Control Get IDs

The Greybus control get IDs Operation is used to retrieve identifying
information from the Mod.

#### Greybus Control Get IDs Request

The Greybus control get IDs request is sent by the AP to all non-AP
modules. The Greybus control get IDs has no payload.

#### Greybus Control Get IDs Response

The Greybus control Get IDs returns identifying information for mod.

    :label: table-control-get-ids-response

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
     0       unipro_mfg_id   4       Mfg ID          Manufacturer ID
     4       unipro_prod_id  4       Product ID      Product ID
     8       ara_vendor_id   4       Vendor ID       Board Developer ID
    12       ara_prod_id     4       Product ID      Developer Unique Product ID
    16       uuid_low        8       UUID (low)      Low order bits of UUID
    24       uuid_low        8       UUID (high)     High order bits of UUID
    =======  ==============  ======  ============    ===========================

SVC Protocol
------------

The AP Module is required to provide a CPort that uses the SVC Protocol
on an Interface. The AP Module does not have a control connection, but
instead implements the SVC protocol using the reserved Control CPort ID.
At initial power-on, the SVC sets up a *unipro* connection from one of
its CPorts to the AP Module Interface's SVC CPort.

The SVC has direct control over and responsibility for the Endo,
including detecting when modules are present, configuring the *unipro*
switch, powering module Interfaces, and attaching and detaching modules.
The AP Module controls the Endo through operations sent over the SVC
connection. And the SVC informs the AP Module about Endo events (such as
the presence of a new module, or notification of changing power
conditions).

Conceptually, the operations in the Greybus SVC Protocol are:

    int version(u8 offer_major, u8 offer_minor, u8 *major, u8 *minor);

Negotiates the major and minor version of the Protocol used for
communication over the connection.  The SVC offers the
version of the Protocol it supports.  The AP replies with
the version that will be used--either the one offered if
supported or its own (lower) version otherwise.  Protocol
handling code adhering to the Protocol specified herein supports
major version |gb-major|, minor version |gb-minor|.

    int svc_hello(u16 endo_id, u8 intf_id);

This Operation is used at initial power-on, sent by the SVC to
inform the AP of its environment. After version negotiation,
it is the next operation initiated by the SVC sent at
initialization. The descriptor describes details of the endo
environment such as number, placement, and features of interface
blocks, etc.

    int dme_peer_get(u8 intf_id, u16 attribute, u16 selector, u16 *result_code, u32 *value);

This Operation is used by the AP to direct the SVC to perform a
*unipro* DME peer get on its behalf. The SVC returns the value
of the DME attribute requested.

    int dme_peer_set(u8 intf_id, u16 attribute, u16 selector, u32 value, u16 *result_code);

This Operation is used by the AP to direct the SVC to perform a
*unipro* DME peer set on its behalf.

    int route_create(u8 intf1_id, u8 intf2_id);

This Operation is used by the AP to direct the SVC to create
a route for *unipro* traffic between two interfaces.

    int route_destroy(u8 intf1_id, u8 intf2_id);

This Operation is used by the AP to direct the SVC to destroy
a route for *unipro* traffic between two interfaces.

    int intf_device_id(u8 intf_id, u8 device_id);

This operation is used by the AP Module to request that the SVC
associate a device ID with the given Interface.

    int intf_hotplug(u8 intf_id, u32 unipro_mfg_id, u32 unipro_prod_id, u32 ara_vend_id, u32 ara_prod_id);

The SVC sends this to the AP Module to inform it that it has
detected a module on the indicated Interface.  It supplies some information
that describes the module that has been attached.

    int intf_hotunplug(u8 intf_id);

The SVC sends this to the AP Module to tell it that a module is
no longer present on an Interface.

    int intf_reset(u8 intf_id);

The SVC sends this to inform the AP Module that an active
Interface needs to be reset.  This might happen when the SVC has
detected
an error on the link, for example.

    int connection_create(u8 intf1_id, u16 cport1_id, u8 intf2_id, u16 cport2_id, u8 tc, u8 flags);

The AP Module uses this operation to request the SVC set up a
*unipro* connection between CPorts on two Interfaces.

    int connection_destroy(u8 intf1_id, u16 cport1_id, u8 intf2_id, u16 cport2_id);

The AP Module uses this operation to request the SVC tear down a
previously created connection.

### Greybus SVC Operations

All SVC Operations are contained within a Greybus SVC request message.
Every SVC request results in a matching response. The request and
response messages for each SVC Operation are defined below.

Table table-svc-operation-type defines the Greybus SVC Protocol
Operation types and their values. Both the request type and response
type values are shown.

    :label: table-svc-operation-type

    ===========================  =============  ==============
    SVC Operation Type           Request Value  Response Value
    ===========================  =============  ==============
    Invalid                      0x00           0x80
    Interface device ID          0x01           0x81
    Interface hotplug            0x02           0x82
    Interface hot unplug         0x03           0x83
    Interface reset              0x04           0x84
    Connection create            0x05           0x85
    Connection destroy           0x06           0x86
    Protocol Version             0x07           0x87
    SVC Hello                    0x08           0x88
    DME peer get                 0x09           0x89
    DME peer set                 0x0a           0x8a
    Route create                 0x0b           0x8b
    Route destroy                0x0c           0x8c
    (all other values reserved)  0x0d..0x7f     0x8d..0xff
    ===========================  =============  ==============

### Greybus SVC Protocol Version Operation

The Greybus SVC Protocol version operation allows the Protocol handling
software on both ends of a connection to negotiate the version of the
SVC Protocol to use. It is sent by the SVC at initial power-on.

#### Greybus SVC Protocol Version Request

Table table-svc-version-request defines the Greybus SVC Protocol version
request payload. The request supplies the greatest major and minor
version of the SVC Protocol supported by the SVC.

    :label: table-svc-version-request

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      Offered SVC Protocol major version
    1        version_minor   1       |gb-minor|      Offered SVC Protocol minor version
    =======  ==============  ======  ==========      ===========================

#### Greybus SVC Protocol Version Response

The Greybus SVC Protocol version response payload contains two one-byte
values, as defined in table table-svc-protocol-version-response. A
Greybus SVC controller adhering to the Protocol specified herein shall
report major version |gb-major|, minor version |gb-minor|.

    :label: table-svc-protocol-version-response

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      SVC Protocol major version
    1        version_minor   1       |gb-minor|      SVC Protocol minor version
    =======  ==============  ======  ==========      ===========================

### Greybus SVC Hello Operation

The Greybus SVC Hello Operation is sent by the SVC to the AP at power-on
to inform the AP of its environment.

#### Greybus SVC Hello Request

Table table-svc-hello-request defines the Greybus SVC Hello Request
payload. This Operation is used at initial power-on, sent by the SVC to
inform the AP of its environment. After version negotiation, it is the
next Operation sent by the SVC sent at initialization. The descriptor
describes details of the endo environment and location of the AP
interface.

    :label: table-svc-hello-request

    =======  ==============  ===========  ===============  ===========================
    Offset   Field           Size         Value            Description
    =======  ==============  ===========  ===============  ===========================
    0        endo_id         2            Endo ID          Endo ID
    2        intf_id         1            AP Interface ID  AP Interface ID
    =======  ==============  ===========  ===============  ===========================

#### Greybus SVC Hello Response

The Greybus SVC Hello response contains no payload.

### Greybus SVC DME Peer Get Operation

The Greybus SVC DME Peer Get Operation is sent by the SVC to the AP to
direct the SVC to perform a *unipro* DME Peer Get on an Interface.

#### Greybus SVC DME Peer Get Request

Table table-dme-peer-get-request defines the Greybus SVC DME Peer Get
Request payload. This request may be sent by the AP to query specific
attributes located in the *unipro* stack of an Interface. The SVC
returns the value of the DME attribute requested.

    :label: table-dme-peer-get-request

    =======  ==============  ===========  ===============  ===========================
    Offset   Field           Size         Value            Description
    =======  ==============  ===========  ===============  ===========================
    0        intf_id         1            Interface ID     Interface ID
    1        attr            2            DME Attribute    *unipro* DME Attribute
    3        selector        2            Selector index   *unipro* DME selector
    =======  ==============  ===========  ===============  ===========================

#### Greybus SVC DME Peer Get Response

Table table-dme-peer-get-response defines the Greybus SVC DME Peer Get
Response payload. The Greybus DME Peer Get response contains the
ConfigResultCode as defined in the *unipro* specification, as well as
the value of the attribute, if applicable.

    :label: table-dme-peer-get-response

    =======  ==============  ===========  ================  =========================================
    Offset   Field           Size         Value             Description
    =======  ==============  ===========  ================  =========================================
    0        result_code     2            ConfigResultCode  *unipro* DME Peer Get ConfigResultCode
    2        attr_value      4            Attribute value   *unipro* DME Peer Get DME Attribute value
    =======  ==============  ===========  ================  =========================================

### Greybus SVC DME Peer Set Operation

The Greybus SVC DME Peer Set Operation is sent by the SVC to the AP to
direct the SVC to perform a *unipro* DME\_PEER\_SET on an Interface.

#### Greybus SVC DME Peer Set Request

Table table-dme-peer-set-request defines the Greybus SVC DME Peer Set
Request payload. This request may be sent by the AP to set specific
attributes located in the *unipro* stack of an Interface.

    :label: table-dme-peer-set-request

    =======  ==============  ===========  ===============  ===================================
    Offset   Field           Size         Value            Description
    =======  ==============  ===========  ===============  ===================================
    0        intf_id         1            Interface ID     Interface ID
    1        attr            2            DME Attribute    *unipro* DME Attribute
    3        selector        2            Selector index   *unipro* DME selector
    5        value           4            Attribute value  *unipro* DME Attribute value to set
    =======  ==============  ===========  ===============  ===================================

#### Greybus SVC DME Peer Set Response

Table table-dme-peer-set-response defines the Greybus SVC DME Peer Set
Response payload. The Greybus DME Peer Set response contains the
ConfigResultCode as defined in the *unipro* specification.

    :label: table-dme-peer-set-response

    =======  ==============  ===========  ================  =========================================
    Offset   Field           Size         Value             Description
    =======  ==============  ===========  ================  =========================================
    0        result_code     2            ConfigResultCode  *unipro* DME Peer Set ConfigResultCode
    =======  ==============  ===========  ================  =========================================

### Greybus SVC Route Create Operation

The Greybus SVC Protocol Route Create Operation allows the AP Module to
request a route be established for *unipro* traffic between two
Interfaces.

#### Greybus SVC Route Create Request

Table table-svc-route-create-request defines the Greybus SVC Route
Create request payload. The request supplies the Interface IDs of two
Interfaces to be connected.

    :label: table-svc-route-create-request

    =======  ==============  ======  ==========  ===========================
    Offset   Field           Size    Value       Description
    =======  ==============  ======  ==========  ===========================
    0        intf1_id        1       intf1_id    First Interface
    1        intf2_id        1       intf2_id    Second Interface
    =======  ==============  ======  ==========  ===========================

#### Greybus SVC Route Create Response

The Greybus SVC Protocol Route Create response contains no payload.

### Greybus SVC Route Destroy Operation

The Greybus SVC Protocol Route Destroy Operation allows the AP Module to
request a route be torn down for *unipro* traffic between two
Interfaces.

#### Greybus SVC Route Destroy Request

Table table-svc-route-destroy-request defines the Greybus SVC Route
Create request payload. The request supplies the Interface IDs of two
Interfaces to be disconnected.

    :label: table-svc-route-destroy-request

    =======  ==============  ======  ==========  ===========================
    Offset   Field           Size    Value       Description
    =======  ==============  ======  ==========  ===========================
    0        intf1_id        1       intf1_id    First Interface
    1        intf2_id        1       intf2_id    Second Interface
    =======  ==============  ======  ==========  ===========================

#### Greybus SVC Route Destroy Response

The Greybus SVC Protocol Route Destroy response contains no payload.

### Greybus SVC Interface Device ID Operation

The Greybus SVC Interface Device ID Operation is used by the AP Module
to request the SVC associate a device id with an Interface. The device
id is used by the *unipro* switch to determine how packets should be
routed through the network. The AP Module is responsible for managing
the mapping between Interfaces and UniPro device ids. Note that the SVC
always uses device ID 0, and the AP Module always uses device ID 1.

#### Greybus SVC Interface Device ID Request

Table table-svc-device-id-request defines the Greybus SVC Interface
Device ID Request payload.

The Greybus SVC Interface Device ID Request shall only be sent by the AP
Module to the SVC. It supplies the device ID that the SVC will associate
with the indicated Interface. The AP Module can remove the association
of an Interface with a device ID by assigning device ID value 0. The AP
shall not assign a (non-zero) device ID to an Interface that the SVC has
already associated with an Interface, and shall not clear the device ID
of an Interface that has no device ID assigned.

Note that assigning a device ID to an Interface does not cause the SVC
to set up any routes for that device ID. Routes are set up only as
needed when a connection involving a device ID are created, and removed
when an Interface's last connection is destroyed.

    :label: table-svc-device-id-request

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
    0        intf_id         1       Interface ID    Interface ID whose device ID is being assigned
    1        device_id       1       Device ID       *unipro* device ID for Interface
    =======  ==============  ======  ============    ===========================

#### Greybus SVC Interface Device ID Response

The Greybus SVC Interface Device ID response message contains no
payload.

### Greybus SVC Interface Hotplug Operation

When the SVC first detects that a module is present on an Interface, it
sends an Interface Hotplug Request to the AP Module. The hotplug request
is sent after the Interface's *unipro* link has been established. The
request includes some additional information known by the SVC about the
discovered Interface (such as the vendor and product ID).

#### Greybus SVC Interface Hotplug Request

Table table-svc-hotplug-request defines the Greybus SVC Interface
Hotplug Request payload.

The Greybus SVC hotplug request is sent only by the SVC to the AP
Module. The Interface ID informs the AP Module which Interface now has a
module present, and supplies information (such as the vendor and model
numbers) the SVC knows about the Interface. Exactly one hotplug event
shall be sent by the SVC for a module when it has been inserted (or if
it was found to be present at initial power-on).

    :label: table-svc-hotplug-request

    ======  ==============  ====  ============  =======================================
    Offset  Field           Size  Value         Description
    ======  ==============  ====  ============  =======================================
    0       intf_id         1     Interface ID  Interface that now has a module present
    1       unipro_mfg_id   4     *unipro* VID  *unipro* DDB Level 1 Manufacturer ID
    5       unipro_prod_id  4     *unipro* PID  *unipro* DDB Level 1 Product ID
    9       ara_vend_id     4     Ara VID       Ara Vendor ID
    13      ara_prod_id     4     Ara PID       Ara Product ID
    ======  ==============  ====  ============  =======================================

#### Greybus SVC Interface Hotplug Response

The Greybus SVC hotplug response message contains no payload.

### Greybus SVC Interface Hot Unplug Operation

The SVC sends this to the AP Module to tell it that an Interface that
was previously the subject of an Interface Hotplug Operation is no
longer present. The SVC sends exactly Interface one hot unplug event to
the AP Module when this occurs.

#### Greybus SVC Interface Hot Unplug Request

Table table-svc-hot-unplug-request defines the Greybus SVC Interface Hot
Unplug Request payload.

The Greybus SVC hot unplog request is sent only by the SVC to the AP
Module. The Interface ID informs the AP which Interface no longer has a
module attached to it. The SVC shall ensure the hotplug event for the
Interface has been successfully delivered to the AP Module before
sending a hot unplug.

    :label: table-svc-hot-unplug-request

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
    0        intf_id         1       Interface ID    Interface that no longer has an attached module
    =======  ==============  ======  ============    ===========================

#### Greybus SVC Interface Hot Unplug Response

The Greybus SVC hot unplug response message contains no payload.

### Greybus SVC Interface Reset Operation

The SVC sends this to the AP Module to request it reset the indicated
link.

#### Greybus SVC Interface Reset Request

Table table-svc-reset-request defines the Greybus SVC Interface Reset
Request payload.

The Greybus SVC Interface Reset Request is sent only by the SVC to the
AP Module. The Interface ID informs the AP Module which Interface needs
to be reset.

    :label: table-svc-reset-request

    =======  ==============  ======  ============    ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ============    ===========================
    0        intf_id         1       Interface ID    Interface to reset
    =======  ==============  ======  ============    ===========================

#### Greybus SVC Interface Reset Response

The Greybus SVC Interface Reset response message contains no payload.

### Greybus SVC Connection Create Operation

The AP Module sends this Operation to the SVC to request that it
establish a *unipro* connection between the two indicated CPorts. The
SVC uses each (intf\_id, cport\_id) pair to determine the *unipro*
(DeviceID\_Enc, CPortID\_Enc) it represents. It is an error to attempt
to create a connection using a CPort that is already in use in another
connection.

#### Greybus SVC Connection Create Request

Table table-svc-connection-create-request defines the Greybus SVC
Connection Create Request payload.

The Greybus SVC connection create request is sent only by the AP Module
to the SVC. The first Interface ID and first CPort ID define one end of
the connection to be established, and the second Interface ID and CPort
ID define the other end.

CPort flags can be specified as a bitwise-or of flags in *flags*, and
are defined in table table-svc-connection-create-request-flags. When
set, the corresponding feature is enabled. For example, specifying CSD
enables *unipro* Controlled Segment Dropping.

    :label: table-svc-connection-create-request

    =======  ==============  ======  ==================  ===========================
    Offset   Field           Size    Value               Description
    =======  ==============  ======  ==================  ===========================
    0        intf1_id        1       Interface ID        First Interface
    1        cport1_id       2       CPort ID            CPort on first Interface
    3        intf2_id        1       Interface ID        Second Interface
    4        cport2_id       2       CPort ID            CPort on second Interface
    6        tc              1       Traffic class       *unipro* traffic class
    7        flags           1       Connection flags    *unipro* connection flags
    =======  ==============  ======  ==================  ===========================

    :label: table-svc-connection-create-request-flags

    =======  ==============  =====================================
    Value    Flag            Description
    =======  ==============  =====================================
    0x01     E2EFC           *unipro* L4 End-to-End Flow Control
    0x02     CSD             *unipro* Controlled Segment Dropping
    0x04     CSV             *unipro* CPort Safety Valve
    =======  ==============  =====================================

#### Greybus SVC Connection Create Response

The Greybus SVC connection create response message contains no payload.

### Greybus SVC Connection Destroy Operation

The AP Module sends this to the SVC to request that a connection that
was previously set up by a Connection Create Operation be torn down. The
AP Module shall have sent Disconnected Control Operations to the two
Interfaces prior to this call. It is an error to attempt to destroy a
connection more than once.

#### Greybus SVC Connection Destroy Request

Table table-svc-connection-destroy-request defines the Greybus SVC
Connection Destroy Request payload.

The Greybus SVC connection destroy request is sent only by the AP Module
to the SVC. The two (Interface ID, CPort ID) pairs define the connection
to be destroyed.

    :label: table-svc-connection-destroy-request

    =======  ==============  ======  ==================  ===========================
    Offset   Field           Size    Value               Description
    =======  ==============  ======  ==================  ===========================
    0        intf1_id        1       Interface ID        First Interface
    1        cport1_id       2       CPort ID            CPort on first Interface
    3        intf2_id        1       Interface ID        Second Interface
    4        cport2_id       2       CPort ID            CPort on second Interface
    =======  ==============  ======  ==================  ===========================

#### Greybus SVC Connection Destroy Response

The Greybus SVC connection destroy response message contains no payload.

Firmware Protocol
-----------------

The Greybus Firmware Protocol is used by a module's bootloader to
communicate with the AP and download firmware executables via *unipro*
when a module does not have its own firmware pre-loaded.

The operations in the Greybus Firmware Protocol are:

    int version(u8 offer_major, u8 offer_minor, u8 *major, u8 *minor);

Negotiates the major and minor version of the Protocol used for
communication over the connection.  The AP sends the request offering the
version of the Protocol it supports.  The module responds with the version
that shall be used--either the one offered if supported, or its own lower
version.  Protocol handling code adhering to the Protocol specified here
supports major version |gb-major|, minor version |gb-minor|.

    int firmware_size(u8 stage, u32 *size);

The module requests from the AP the size of the firmware it must
load, specifying the stage of the boot sequence for which the module is
requesting firmware.  The AP then locates a suitable firmware blob,
associates that firmware blob with the requested boot stage until it next
receives a firmware size request, and responds with the blob's size in
bytes, which must be nonzero.

    int get_firmware(u32 offset, u32 size, void *data);

The module requests a finite stream of bytes in the firmware blob
from the AP, passing its current offset into the firmware blob, and the size
of the stream it currently needs.  The AP responds with exactly the number
of bytes requested, taken from the firmware blob currently associated with
this connection at the specified offset.

    int ready_to_boot(u8 status);

The module implementing the Protocol requests permission from the AP to jump
into the firmware blob it has loaded.  The request sent to the AP includes a
status indicating whether the retrieved firmware blob is valid and secure,
valid but insecure, or invalid.  The AP decides whether to permit the module
to boot in its current condition: if so, it sends a success code in its
response's status byte, otherwise it sends an error code in its response's
status byte.

### Greybus Firmware Operations

Table table-firmware-operation-type describes the Greybus firmware
operation types and their values. A message type consists of an
operation type combined with a flag (0x80) indicating whether the
operation is a request or a response.

    :label: table-firmware-operation-type

    ===========================  =============  ==============
    Firmware Operation Type      Request Value  Response Value
    ===========================  =============  ==============
    Invalid                      0x00           0x80
    Protocol Version             0x01           0x81
    Firmware Size                0x02           0x82
    Get Firmware                 0x03           0x83
    Ready to Boot                0x04           0x84
    (all other values reserved)  0x05..0x7f     0x85..0xff
    ===========================  =============  ==============

### Greybus Firmware Protocol Version Operation

The Greybus firmware Protocol version operation allows the Protocol
handling software on both ends of a connection to negotiate the version
of the firmware Protocol to use.

#### Greybus Firmware Protocol Version Request

Table table-firmware-version-request defines the Greybus firmware
version request payload. The request supplies the greatest major and
minor version of firmware Protocol supported by the sender (the AP).

    :label: table-firmware-version-request

    ======  =====   ====    ==========  =======================================
    Offset  Field   Size    Value       Description
    ======  =====   ====    ==========  =======================================
    0       major   1       |gb-major|  Offered firmware Protocol major version
    1       minor   1       |gb-minor|  Offered firmware Protocol minor version
    ======  =====   ====    ==========  =======================================

#### Greybus Firmware Protocol Version Response

Table table-firmware-version-response defines the Greybus firmware
version response payload. A Greybus module implementing the Protocol
described herein shall report major version |gb-major|, minor version
|gb-minor|.

    :label: table-firmware-version-response

    ======  =====   ====    ==========  ===============================
    Offset  Field   Size    Value       Description
    ======  =====   ====    ==========  ===============================
    0       major   1       |gb-major|  Firmware Protocol major version
    1       minor   1       |gb-minor|  Firmware Protocol minor version
    ======  =====   ====    ==========  ===============================

### Greybus Firmware Firmware Size Operation

The Greybus Firmware firmware size operation allows the requestor to
submit a boot stage to the AP, so that the AP can associate a firmware
blob with that boot stage and respond with its size. The AP keeps the
firmware blob associated with the boot stage until it receives another
Firmware Size Request on the same connection, but is not required to
send identical firmware blobs in response to different requests with
identical boot stages, even to the same module.

#### Greybus Firmware Firmware Size Request

Table table-firmware-size-request defines the Greybus firmware size
request payload. The request supplies the boot stage of the module
implementing the Protocol.

    :label: table-firmware-size-request

    ======  =========  ====  ======  ===============================================
    Offset  Field      Size  Value   Description
    ======  =========  ====  ======  ===============================================
    0       stage      1     Number  :ref:`firmware-boot-stages`
    ======  =========  ====  ======  ===============================================

#### Greybus Firmware Boot Stages

Table table-firmware-boot-stages defines the boot stages whose firmware
can be requested from the AP via the Protocol.

    :label: table-firmware-boot-stages

    ================  ======================================================  ==========
    Boot Stage        Brief Description                                       Value
    ================  ======================================================  ==========
    BOOT_STAGE_ONE    Reserved for the boot ROM.                              0x01
    BOOT_STAGE_TWO    Firmware package to be loaded by the boot ROM.          0x02
    BOOT_STAGE_THREE  Module personality package loaded by Stage 2 firmware.  0x03
    |_|               (Reserved Range)                                        0x04..0xFF
    ================  ======================================================  ==========

#### Greybus Firmware Firmware Size Response

Table table-firmware-size-response defines the Greybus firmware size
response payload. The response supplies the size of the AP's firmware
blob for the module implementing the Protocol.

    :label: table-firmware-size-response

    ======  =====  ====  ======  =========================
    Offset  Field  Size  Value   Description
    ======  =====  ====  ======  =========================
    0       size   4     Number  Size of the blob in bytes
    ======  =====  ====  ======  =========================

### Greybus Firmware Get Firmware Operation

The Greybus Firmware get firmware operation allows the requestor to
retrieve a stream of bytes at an offset within the firmware blob from
the AP. The AP responds with the requested number of bytes from the
connection's associated firmware blob at the requested offset, or with
an error status without payload if no firmware blob has yet been
associated with this connection or if the requested stream size exceeds
the firmware blob's size minus the requested offset.

#### Greybus Firmware Get Firmware Request

Table table-firmware-get-firmware-request defines the Greybus Firmware
get firmware request payload. The request specifies an offset into the
firmware blob, and the size of the stream of bytes requested. The stream
size requested must be less than or equal to the size given by the most
recent firmware size response (firmware-size-response) minus the offset;
when it is not, the AP shall signal an error in its response. The module
is responsible for tracking its offset into the firmware blob as needed.

    :label: table-firmware-get-firmware-request

    ======  ====== ====  ======  =================================
    Offset  Field  Size  Value   Description
    ======  ====== ====  ======  =================================
    0       offset 4     Number  Offset into the firmware blob
    4       size   4     Number  Size of the byte stream requested
    ======  ====== ====  ======  =================================

#### Greybus Firmware Get Firmware Response

Table table-firmware-get-firmware-response defines the Greybus Firmware
get firmware response payload. The response includes the stream of bytes
requested by the module. In the case that the AP cannot fulfill the
request, such as when the requested stream size was greater than the
total size of the firmware blob, it shall signal an error in the status
byte of the response header.

    :label: table-firmware-get-firmware-response

    ======  =====  ====== ======  =================================
    Offset  Field  Size   Value   Description
    ======  =====  ====== ======  =================================
    4       data   *size* Data    Data from the firmware blob
    ======  =====  ====== ======  =================================

### Greybus Firmware Ready to Boot Operation

The Greybus Firmware ready to boot operation lets the requesting module
notify the AP that it has successfully loaded the connection's
currently-associated firmware blob and is able to hand over control of
the processor to that blob, indicating the status of its firmware blob.
The AP shall then send a response empty of payload, indicating via the
header's status byte whether or not it permits the module to continue
booting.

The module shall send a ready to boot request only when it has
successfully loaded a firmware blob and can execute that firmware.

#### Greybus Firmware Ready to Boot Request

Table table-firmware-ready-to-boot-request defines the Greybus Firmware
ready to boot request payload. The request gives the security status of
its firmware blob.

    :label: table-firmware-ready-to-boot-request

    ======  ======  ====  ======  ===========================
    Offset  Field   Size  Value   Description
    ======  ======  ====  ======  ===========================
    0       status  1     Number  :ref:`firmware-blob-status`
    ======  ======  ====  ======  ===========================

#### Greybus Firmware Ready to Boot Firmware Blob Status

Table table-firmware-blob-status defines the constants by which the
module can indicate the status of its firmware blob to the AP in a
Greybus Firmware Ready to Boot Request.

    :label: table-firmware-blob-status

    ====================  ====================================  ============
    Firmware Blob Status  Brief Description                     Status Value
    ====================  ====================================  ============
    BOOT_STATUS_INVALID   Firmware blob could not be validated  0x00
    BOOT_STATUS_INSECURE  Firmware blob is valid but insecure   0x01
    BOOT_STATUS_SECURE    Firmware blob is valid and secure     0x02
    |_|                   (Reserved Range)                      0x03..0xFF
    ====================  ====================================  ============

#### Greybus Firmware Ready to Boot Response

If the AP permits the module to boot in its current status, the Greybus
Firmware Ready to Boot response message shall have no payload. In the
case that the AP forbids the module from booting, it shall signal an
error in the status byte of the response message's header.

Mods Exensions
==============

The general goal is to keep as much commonality with upstream projects,
but ultimately some additions are required to fully support Mods.

Sensors-Ext Protocol
--------------------

This section defines the operations used on a connection implementing
the Greybus Sensors-Ext Protocol. This Protocol allows an AP Module to
manage a sensor device present on a Module. The Protocol mirrors closely
the Android HAL sensors interface.

The Greybus Sensors-Ext Protocol uses *descriptors* and *reports* to
interact with a Module containing sensor devices. A Sensors-Ext
Descriptor defines all the capabilities of a sensor device on the
Module. When the Module has acquired data from one or more sensors
devices that it wishes to report, it sends a Report to the AP Module.

A Module containing sensor devices may need to store sensor calibration
data. The data could be stored on the Module, or the AP Module may be
used to persist the calibration data using a protocol such as the
UniPro-based flash storage standard - UFS. The exact mechanism used is
outside the scope of this Sensors-Ext Protocol.

### Greybus Sensors-Ext Operations

Conceptually, the operations in the Greybus Sensors-Ext Protocol are:

    int version(u8 offer_major, u8 offer_minor, u8 *major, u8 *minor);

Negotiates the major and minor version of the Protocol used for
communication over the connection. The sender offers the
version of the Protocol it supports. The receiver replies with
the version that will be used--either the one offered if
supported, or its own (lower) version otherwise. Protocol
handling code adhering to the Protocol specified herein supports
major version |gb-major|, minor version |gb-minor|.

    int get_sensor_count(u8 *count);

This Operation is used by the AP to obtain a count of the number of sensors
supported by the Module. The count will include the physical and virtual
sensors provided by the module. This operation is only initiated by the AP.

    int get_sensor_info(u8 sensor_id, u8 **sensor_info);

This Operation is used by the AP to obtain the attributes associated with
a specific sensor. This operation is only initiated by the AP.

    int start_reporting(u8 sensor_id, u64 sampling_period, u64 max_report_latency);

This Operation is used by the AP to enable data reporting from a sensor
device, or to modify the reporting parameters. This operation is only
initiated by the AP Module. See the :ref:`sensors-start-reporting-request`
section for details on the units used.

    int flush(u8 sensor_id);

This Operation is used by the AP module to request a Module to flush
any readings accumulated in its FIFO for the corresponding sensor.
This Operation is only initiated by the AP Module.

    int stop_reporting(u8 sensor_id);

This Operation is used by the AP Module to disable data reporting from a
sensor device. This operation is only initiated by the AP Module.

    int time_sync(u64* current_time, u8* precision, u8* accuracy, u8 flags);

This Operation is used between an AP Module and another Module containing
sensor devices to synchronize the clock used to time stamp sensor events.

    int mode(u8 flags);

This Operation is used by the AP Module to inform a peer that it is
switching operating modes, such as entering or exiting a low power state.

### Greybus Sensors-Ext Message Types

Table table-sensors-ext-operation-type describes the Greybus sensors
operation types and their values (refer to the Operation Message Header
format in Table table-operation-message-header). A message type consists
of an operation type combined with a flag (0x80) indicating whether the
operation is a request or a response. Some of the operation requests or
responses may be used in a unidirectional mode.

    :label: table-sensors-ext-operation-type

    ===========================  =============  ==============
    Sensors-Ext Operation Type   Request Value  Response Value
    ===========================  =============  ==============
    Invalid                      0x00           0x80
    Protocol Version             0x01           0x81
    Sensor Count                 0x02           0x82
    Sensor Info                  0x03           0x83
    Start Reporting              0x04           0x84
    Report                       0x05           0x85
    Flush                        0x06           0x86
    Stop Reporting               0x07           0x87
    Time Sync                    0x08           0x88
    Mode                         0x09           0x89
    (all other values reserved)  0x0a..0x7f     0x8a..0xff
    ===========================  =============  ==============

### Greybus Sensors-Ext Protocol Version Operation

The Greybus sensors Protocol Version operation allows the Protocol
handling software on both ends of a connection to negotiate the version
of the sensors Protocol to use.

#### Greybus Sensors-Ext Protocol Version Request

Table table-sensors-ext-version-request defines the Greybus sensors
version request payload. The request supplies the greatest major and
minor version of the sensors Protocol supported by the sender.

    :label: table-sensors-ext-version-request

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      Offered sensors Protocol major version
    1        version_minor   1       |gb-minor|      Offered sensors Protocol minor version
    =======  ==============  ======  ==========      ===========================

#### Greybus Sensors-Ext Protocol Version Response

The Greybus sensors Protocol version response payload contains two
one-byte values, as defined in Table
table-sensors-ext-protocol-version-response. A Greybus sensors module
adhering to the Protocol specified herein shall report major version
|gb-major|, minor version |gb-minor|.

    :label: table-sensors-ext-protocol-version-response

    =======  ==============  ======  ==========      ===========================
    Offset   Field           Size    Value           Description
    =======  ==============  ======  ==========      ===========================
    0        version_major   1       |gb-major|      Sensors-Ext Protocol major version
    1        version_minor   1       |gb-minor|      Sensors-Ext Protocol minor version
    =======  ==============  ======  ==========      ===========================

### Greybus Sensors-Ext Sensor Count Operation

The Greybus sensors Sensor Count operation allows the AP Module to
obtain a count of the number of sensors supported by the Module.

#### Greybus Sensors-Ext Sensor Count Request

The Sensor Count request message has no payload.

#### Greybus Sensors-Ext Sensor Count Response

The Sensor Count response message serves two purposes. First, it
provides a count for the number of sensors supported by the Module.
Second, it effectively assigns an ID to each sensor. Table
table-sensors-ext-count-response defines the Greybus Sensors Count
Response.

    :label: table-sensors-ext-count-response

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        count               1       Number          The number of sensors available
    =======  ==================  ======  =============   ===========================

*count* is the number of sensors provided by the module. This *count* must
include both the physical and virtual sensors.

A Sensor ID is a unique ID that identifies a particular sensor supported by the
Module. Sensor ID's shall be integer values in the range [0, *count*).

The Module must be able to supply a unique Sensor Descriptor for each Sensor ID
when requested to do so with the Greybus Sensor Info Request described below.

*count* is the number of sensors provided by the module. This *count*
must include both the physical and virtual sensors.

A Sensor ID is a unique ID that identifies a particular sensor supported
by the Module. Sensor ID's shall be integer values in the range [0,
*count*).

The Module must be able to supply a unique Sensor Descriptor for each
Sensor ID when requested to do so with the Greybus Sensor Info Request
described below.

### Greybus Sensors-Ext Sensor Info Operation

The Sensor Info operation allows the Protocol handling software to
obtain the attributes associate with a particular sensor provided by a
Module.

#### Greybus Sensors-Ext Sensor Info Request

Table table-sensors-ext-info-request defines the Greybus Sensor Info
Request.

    :label: table-sensors-ext-info-request

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        sensor_id           1       Sensor ID       The handle of the sensor requested
    =======  ==================  ======  =============   ===========================

*sensor_id* is the ID of the sensor whose attributes are requested. This value
must be less than the *count* obtained with the Greybus Sensor Count Operation
defined above.


*sensor\_id* is the ID of the sensor whose attributes are requested.
This value must be less than the *count* obtained with the Greybus
Sensor Count Operation defined above.

#### Greybus Sensors-Ext Sensor Info Response

Table table-sensors-ext-info-response defines the Greybus Sensor Info
Response.

    :label: table-sensors-ext-info-response

    ======  ===============  =================  =============  =============================
    Offset  Field            Size               Value          Description
    ======  ===============  =================  =============  =============================
    0       version          4                  Number         Version of the hardware part + driver
    4       type             4                  Number         The sensor's type
    8       max_range        4                  Number         Maximum range of this sensor's value in SI units
    12      resolution       4                  Number         Smallest difference between two values
    16      power            4                  Number         Power consumption in :math:`\mu A`
    20      min_delay        4                  Signed Number  Minimum sample period allowed (in :math:`\mu s`)
    24      max_delay        4                  Number         Maximum delay (in :math:`\mu s`) between two events
    28      fifo_rec         4                  Number         FIFO reserved event count
    32      fifo_mec         4                  Number         FIFO maximum event count
    36      flags            4                  Number         Flags for the sensor
    40      scale_int        4                  Signed Number  Scaling applied to data to convert to SI units
    44      scale_nano       4                  Number         Scaling applied to data to convert to SI units
    48      offset_int       4                  Signed Number  Offset applied to data to convert to SI units
    52      offset_nano      4                  Number         Offset applied to data to convert to SI units
    56      channels         1                  Number         Count of values that comprise a sensor reading
    57      (pad)            3                  0              Reserved (pad to 4 byte boundary)
    60      name_len         2                  Number         Length of the *name* string in bytes
    62      name             128                UTF-8          Sensor name
    190     vendor_len       2                  Number         Length of the *vendor* string in bytes
    192     vendor           128                UTF-8          Sensor vendor
    320     string_type_len  2                  Number         Length of the *string_type* string in bytes
    322     string_type      256                UTF-8          The type of this sensor as a string
    ======  ===============  =================  =============  =============================

*version* is the version of the hardware part and driver. The value of
this field must increase when the driver is updated in a way that
changes the output of this sensor. This is important for fused sensors
when the fusion algorithm is updated.

*type* is the sensor's type as defined in Table table-sensors-ext-types.
If the sensor doesn't match one of the given types, a value of
0x00000000 must be used and the *string\_type* must be used to describe
the type.

*max\_range* is the maximum range of this sensor's values. After
application of *scale* and *offset* to this number, the result will be
in SI units.

*resolution* is the smallest difference between two values reported by
this sensor. After application of *scale* and *offset* to this number,
the result will be in SI units.

*power* is a rough estimate of this sensor's power consumption in
\f$\mu A\f$.

*min\_delay* depends on the reporting mode. For continuous sensors, this
value defines the minimum sample period allowed (in \f$\mu s\f$). This is
the highest frequency supported by the sensor. For on-change sensors,
this value must be set to 0. For one-shot sensors, this value must be
set to -1.

*max\_delay* is the delay between two sensor events (in \f$\mu s\f$)
corresponding to the lowest frequency that this sensor supports.

*fifo\_rec* is the number of events reserved for this sensor in the
batch mode FIFO. If there is a dedicated FIFO for this sensor, then this
is the size of this FIFO. If the FIFO is shared with other sensors, this
is the size reserved for that sensor and it can be zero.

*fifo\_mec* is the maximum number of events of this sensor that could be
batched. This is especially relevant when the FIFO is shared between
several sensors; this value is then set to the size of that FIFO.

*flags* is a combination of 0 or more flags from Table
table-sensors-ext-flags. **Continuous** sensors generate events at a
constant rate defined by *sampling\_period* in Table
table-sensors-ext-start-reporting-request. **On-change** sensors
generate events only if the measured value has changed. Activating an
on-change sensor will first report the event immediately when the sensor
is activated and then once the reported value changes. The
*sampling\_period* is used to set the minimum time between consecutive
events, meaning an event should not be generated until
*sampling\_period* has elapsed since the last event, even if the value
has changed since then. If the value changed, an event must be generated
as soon as *sampling\_period* has elapsed since the last event.
**One-shot** sensors must first deactivate themselves upon detection of
an event, and then report the event (the order is important as it avoids
race conditions). No other event is sent until the sensor is
reactivated. **Special** sensors have a reporting mode that depends on
the particular sensor generating the events. For example, a *step
detector* generates events each time a step is taken by the user (each
time the foot hits the ground) and a *tilt detector* generates an event
each time a tilt event is detected.

*scale\_int* (*i*) and *scale\_nano* (*n*) are combined to form a
*scale* (*s*) value as follows:

\f[
s = \begin{cases} i + n * 10^{-9}, \quad \mbox{if } i \ge 0 \qquad
i - n * 10^{-9}, \quad \mbox{if } i < 0 \end{cases}
\f]

*scale* is the scaling that must applied to the raw sensor data to
convert it to a Float in SI units. \f$(r + o) * s\f$ is the value in SI
units, *r* is the raw reading, *o* is the *offset*, *s* is the *scale*.

*offset\_int* (*i*) and *offset\_nano* (*n*) are combined to form an
*offset* (*o*) value as follows:

\f[
o = \begin{cases} i + n * 10^{-9}, \quad \mbox{if } i \ge 0 \qquad
i - n * 10^{-9}, \quad \mbox{if } i < 0 \end{cases}
\f]

*channels* specifies the number of values reported by this sensor for
each reading. For example, a 3-axis accelerometer would report 3 values
(x, y, z) for each reading. A sensor that has more *channels* than the
maximum number of values reportable (16 values on Android M) will have
the unreportable values silently dropped.

*name\_len* is the length of the *name* string in bytes. It must be
non-zero.

*name* is a user-visible string that represents the sensor. This string
should contain the part name of the underlying sensor, the type of the
sensor, and whether it is a wake-up sensor. For example "BMP280 Wake-up
Barometer".

*vendor\_len* is the length of the *vendor* string in bytes. It must be
non-zero.

*vendor* provides the name of the sensor vendor.

*string\_type\_len* is the length of the *string\_type* in bytes. If set
to 0, the *string\_type* and following padding is omitted.

*string\_type* is the type of the sensor as a string. This an optional
field for standard sensors defined in Table table-sensors-ext-types.
When defining an OEM specific or manufacturer specific sensor, this
field is mandatory and must start with the OEM or manufacturer reverse
domain name. The *string\_type* is used to uniquely identify
non-official sensor types.

    :label: table-sensors-ext-types

    ============================    ==========
    Sensor Type                     Value
    ============================    ==========
    Undefined                       0x00000000
    Accelerometer                   0x00000001
    Geomagnetic field               0x00000002
    Orientation                     0x00000003
    Gyroscope                       0x00000004
    Light                           0x00000005
    Pressure                        0x00000006
    Temperature                     0x00000007
    Proximity                       0x00000008
    Gravity                         0x00000009
    Linear acceleration             0x0000000a
    Rotation vector                 0x0000000b
    Relative humidity               0x0000000c
    Ambient temperature             0x0000000d
    Magnetic field uncalibrated     0x0000000e
    Game rotation vector            0x0000000f
    Gyroscope uncalibrated          0x00000010
    Significant motion              0x00000011
    Step detector                   0x00000012
    Step counter                    0x00000013
    Geomagnetic rotation vector     0x00000014
    Heart rate                      0x00000015
    Tilt detector                   0x00000016
    Wake gesture                    0x00000017
    Glance gesture                  0x00000018
    Pick-up gesture                 0x00000019
    Wrist tilt gesture              0x0000001a
    (reserved)                      0x0000001b..0xffffffff
    ============================    ==========

    :label: table-sensors-ext-flags

    ============================    ==========
    Sensor Flag                     Value
    ============================    ==========
    Continuous mode                 0x00000000
    Non-maskable                    0x00000001
    On-change mode                  0x00000002
    One-shot mode                   0x00000004
    Special reporting mode          0x00000008
    (All other values reserved)     0x00000010..0xffffffff
    ============================    ==========

### Greybus Sensors-Ext Start Reporting Operation

The Greybus Sensors-Ext Start Reporting operation allows the AP Module
to request reports from a specific sensor.

#### Greybus Sensors-Ext Start Reporting Request

Table table-sensors-ext-start-reporting-request defines the Greybus
sensors Start Reporting request payload. This request is only sent from
the AP to a Module. The Start Reporting Request allows the AP to request
either regular or batched sensor reports from the Module. The request
can be used to activate reporting from a sensor, or to modify the
reporting characteristics of a sensor that is already active. A Start
Reporting Request overrides any previous Start Reporting Requests for a
given sensor.

    :label: table-sensors-ext-start-reporting-request

    =======  ==================  ======  ==========      ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  ==========      ===========================
    0        sensor_id           1       Sensor ID       The handle of the sensor to enable
    1        (pad)               3       0               Reserved (pad to 4 byte boundary)
    4        sampling_period     8       Number          Sampling period (in ns) at which the sensor should run
    12       max_report_latency  8       Number          Maximum time (in ns) by which the events can be delayed
    =======  ==================  ======  ==========      ===========================

The meaning of the *sampling\_period* field depends on the specified
sensor's reporting mode:

> -   Continuous: *sampling\_period* is the sampling rate, meaning the
>     rate at which events are generated.
> -   On-change: *sampling\_period* limits the sampling rate of events,
>     meaning events are generated no faster than every
>     *sampling\_period* nanoseconds. There might be periods longer than
>     *sampling\_period* where no event is generated if the measured
>     values do not change for long periods.
> -   One-shot: *sampling\_period* is ignored.

The value of the *sampling\_period* must fall between the *min\_delay*
and *max\_delay* field values from Table table-sensors-ext-info-response
for the corresponding sensor.

*max\_report\_latency* sets the maximum time by which events can be
delayed and stored in the hardware FIFO before being reported to the AP.
A value of 0 should be used to stream data as soon as it is available. A
value greater than 0 can be used to enable sensor data batching.

#### Greybus Sensors-Ext Start Reporting Response

The Greybus sensors Start Reporting response message has no payload. A
*status* value of 0 in the operation message header indicates success.

### Greybus Sensors-Ext Report Operation

The Greybus sensors Report operation allows the Protocol handling
software to receive sensor data reports from a Module. A Module may send
an unsolicited Report Response when the maximum reporting latency has
expired.

#### Greybus Sensors-Ext Report Response

The Report Response is a contiguous block of data that includes the
Report Response Header, and a set of Sensor Data Reports each containing
the data collected from one of the enabled sensors.

The report can be sent either in response to a Flush Request, or as a
*unidirectional* message when the *max\_report\_latency* has expired or
the hardware FIFO is full.

Table table-sensors-ext-report-response-hdr defines the Report Response
Header.

    :label: table-sensors-ext-report-response-hdr

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        sensors             1       Number          The number of Sensor Data Reports present
    1        (pad)               1       Number          Reserved (pad to 2 byte boundary)
    =======  ==================  ======  =============   ===========================

Table table-sensors-ext-report-response-data defines the Sensor Data
Report for a single sensor. It is followed by a variable number of
individual readings, as specified by the *readings* field. Table
table-sensors-ext-report-response-reading defines the Sensor Reading
format.

    :label: table-sensors-ext-report-response-data

    =======  ==================  =============  =========  ===========================
    Offset   Field               Size           Value      Description
    =======  ==================  =============  =========  ===========================
    0        sensor_id           1              Sensor ID  The handle of the sensor being reported
    1        flags               1              Number     Flags for the response
    2        readings            2              Number     The number of readings reported
    4        reference_time      8              Number     The reference timestamp (in ns from UNIX epoch)
    12       reading_data        Variable size  Data       Variable number of readings defined by table-sensors-ext-report-response-reading
    =======  ==================  =============  =========  ===========================

*sensor\_id* is the ID of the sensor reporting readings.

*flags* is a combination of 0 or more flags from Table
table-sensors-ext-report-response-flags. If the response is issued due
to a Flush Request, the **Flushing** flag must be set. The last Report
Response in response to a Flush Request must have the **Flush Complete**
flag set.

*readings* specifies the number of sensor readings that follow and may
be 0 only if the **Flush Complete** flag is set.

*reference\_time* is the reference time for the *time\_delta* in Table
table-sensors-ext-report-response-reading. When the *time\_delta* is
added to this *reference\_time* the result will be the absolute time the
event occurred in ns from UNIX epoch (TBD: Is that UTC or local time?).

*reading\_data* size will depend on how many readings are present, and
on the size of each reading. It should be exactly \f$readings * (2 + channels * 4)\f$ .


    :label: table-sensors-ext-report-response-flags

    ============================    ==========
    Sensor Flag                     Value
    ============================    ==========
    Flushing                        0x01
    Flush complete                  0x02
    (All other values reserved)     0x04..0xff
    ============================    ==========

    :label: table-sensors-ext-report-response-reading

    =======  ==================  ====================  =============   ===========================
    Offset   Field               Size                  Value           Description
    =======  ==================  ====================  =============   ===========================
    0        time_delta          2                     Number          Time offset (in ns) from the *reference_time*
    2        value               *channels* x 4        Signed Number   Data from a single sensor reading
    =======  ==================  ====================  =============   ===========================

*time\_delta* is the time elapsed between the *reference\_time* value in
Table table-sensors-ext-report-response-data and the time the reading
was reported by the sensor.

*value* is an array of *channels* values reported by the sensor. The
order of values must match the order defined in the Android Sensors HAL
interface.

### Greybus Sensors-Ext Flush Operation

The Greybus sensors Flush operation allows the Protocol handling
software to request the flushing of a sensor's hardware FIFO.

#### Greybus Sensors-Ext Flush Request

When the Greybus Sensors-Ext Flush Request is received by a Module, it
shall flush the hardware or software FIFO for the specified sensor(s) by
reporting all the corresponding events in the FIFO using one or more
Greybus Sensors-Ext Report Response. The Module must respond to the
request with at least one Report Response, so if the FIFO is empty, the
*readings* field will be set to 0. Table table-sensors-ext-flush-request
defines the Greybus Sensors-Ext Flush Request.

    :label: table-sensors-ext-flush-request

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        sensor_id           1       Sensor ID       The handle of the sensor to flush
    =======  ==================  ======  =============   ===========================

*sensor\_id* is the ID of the sensor to flush, or the special value of
0xff to request a flush for all currently enabled sensors.

#### Greybus Sensors-Ext Flush Response

The Greybus sensors Flush response message has no payload. A *status*
value of 0 in the operation message header indicates that the Flush
request was successfully processed.

### Greybus Sensors-Ext Stop Reporting Operation

The Greybus sensors Stop Reporting operation allows the Protocol
handling software to disable sensor reporting for a specific sensor.

#### Greybus Sensors-Ext Stop Reporting Request

When the Greybus Sensors-Ext Stop Reporting Request is received by a
Module, it shall stop reporting readings from the specified sensor and
clear the hardware or software FIFO by discarding all the corresponding
events in the FIFO. Table table-sensors-ext-stop-reporting-request
defines the Greybus Sensors-Ext Stop Reporting Request.

    :label: table-sensors-ext-stop-reporting-request

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        sensor_id           1       Sensor ID       The handle of the sensor to stop reporting
    =======  ==================  ======  =============   ===========================

#### Greybus Sensors-Ext Stop Reporting Response

The Greybus sensors Stop Reporting response message has no payload. A
*status* value of 0 in the operation message header indicates that the
Stop Reporting request was successfully processed.

### Greybus Sensors-Ext Time Sync Operation

The Greybus sensors Time Sync operation allows the Protocol handling
sofware to synchronize the time between the AP and a sensors Module. The
request can be initiated by either the AP or the Module, allowing for
pull or push time synchronization.

#### Greybus Sensors-Ext Time Sync Request

The Time Sync Request serves two purposes. It informs the receiver what
the sender's time and time resolution is, and requests the receiver to
respond back with the same information.

Table table-sensors-ext-time-sync defines the Greybus Sensors-Ext Time
Sync Request.

    :label: table-sensors-ext-time-sync

    =======  ==================  ======  =============  ===========================
    Offset   Field               Size    Value          Description
    =======  ==================  ======  =============  ===========================
    0        current_time        8       Number         The current time, in ns from the UNIX epoch.
    8        precision           1       Signed Number  The clock precision.
    9        accuracy            1       Signed Number  The clock accuracy.
    10       flags               1       Number         Flags
    =======  ==================  ======  =============  ===========================

*current_time* is the current time (in ns from the UNIX epoch) of the sender.
This value will roll-over in year 2554.

*precision* is a signed integer that when substituted for *v* in \f$1.2^{v - 61}\f$
yields the precision in seconds of the clock from which *current_time*
is derived.

*accuracy* is a signed integer that when substitued for *v* in \f$1.2^{v - 61}\f$
yields the accuracy in seconds of the clock from which *current_time* is
derived.

The coded value of *precision* or *accuracy* can be obtained by substituting
for *x* in \f$log_{1.2}x + 61\f$ or \f$\frac{log(x)}{log(1.2)} + 61\f$ the
*precision* or *accuracy* value expressed in seconds.

*flags* is a combination of 0 or more flags from Table
table-sensors-ext-tine-sync-flags. When the *Update* flag is set, the
Module must time stamp all events with a clock that uses *current\_time*
as the time base.

    :label: table-sensors-ext-time-sync-flags

    ============================    ==========
    Sensor Time Sync Flag           Value
    ============================    ==========
    Update                          0x01
    (All other values reserved)     0x02..0xff
    ============================    ==========

#### Greybus Sensors-Ext Time Sync Response

The Time Sync Response is identical to the Time Sync Request defined in
Table table-sensors-ext-time-sync.

When responding to a request, *current\_time* field shall be populated
before applying the clock correction provided by the sender (for cases
where the *Update* flag is set). This allows the sender to monitor the
clock drift of the receiver and apply corrections as it deems necessary.

### Greybus Sensors-Ext Mode Operation

The Greybus Sensors-Ext Mode operation allows the AP Module to inform
another module that it is switching operating modes.

#### Greybus Sensors-Ext Mode Request

Table table-sensors-ext-mode-request defines the Mode request payload.
This request is only sent from the AP to a Module.

    :label: table-sensors-ext-mode-request

    =======  ==================  ======  =============   ===========================
    Offset   Field               Size    Value           Description
    =======  ==================  ======  =============   ===========================
    0        flags               1       Number          Operating mode flags from Table table-sensors-ext-mode-flags
    =======  ==================  ======  =============   ===========================

*flags* is a combination of flags from Table
table-sensors-ext-mode-flags. If a Module receives a Mode Request with
the *Mask-sensors* flag set, it must block all further reporting of data
from maskable sensors until a new Mode Request is received with the
*Mask-sensors* flag reset.

    :label: table-sensors-ext-mode-flags

    ============================    ==========
    Sensors-Ext Mode Flags          Value
    ============================    ==========
    Mask-sensors                    0x01
    ============================    ==========

#### Greybus Sensors-Ext Mode Response

The Greybus Mode response message has no payload. A *status* value of 0
in the operation message header indicates success.

Glossary
========

AP Module
    * Application Processor Module.

Application Processor Module
    * A specially designated Module within a Greybus System.

    * An AP Module administers a Greybus System by exchanging SVC Protocol and
      Control Protocol Operations with the SVC.

Bridged PHY Protocol
    * A Protocol which allows Modules to expose functionality to the Greybus
      System which is provided by chipsets using alternative physical
      interfaces than *unipro*, or which do not comply with an existing Device
      Class Protocol.

Connection
    * A bidirectional communication path between exactly two Modules, or
      between a Module and the SVC.

    * There is a *unipro* CPort at each end of a Connection. Each of the two
      CPorts associated with a Connection corresponds to an Interface on a
      distinct Module associated with the connection. Modules exchange data on
      a Connection through transmission and reception of *unipro* Messages,
      according to one of the Protocols defined by the Greybus Specification.

Connection Protocol
    * See Protocol.

Device Class Protocol
    * A Protocol which allow Modules to expose functionality commonly found on
      mobile handsets to the Greybus System, in a manner that abstracts various
      hardware-specific aspects by which that functionality is implemented.

Endo
    * Endoskeleton.

Endoskeleton
    * A phsyical entity within a Greybus System, containing one or more
      *unipro* Switches, exactly one SVC, and a collection of Interfaces. Every
      Module exchanges *unipro* Messages with other elements of a Greybus
      System by physical connection to one or more Interfaces.

Greybus System
    * An implementation of the Project Ara platform which complies with the
      Greybus Specification.

    * A Greybus System is composed of one or more Modules , connected to one
      another and to exactly one SVC by one or more *unipro* Switches contained
      within an Endoskeleton.

Interface
    * An abstract representation of the services provided by a Module at one of
      its Interface Blocks.

Interface Block
    * The physical connections between a Module and an Endoskeleton as defined
      by the Project Ara MDK.

Message Header
    * The Message Header is a common data structure which occurs at offset zero
      of each *unipro* Message containing an individual Greybus Operation's
      Request or Response. Within the Message, the Message Header is followed
      by an optional payload, as defined by the Operation's Protocol.

Module
    * A physical entity within a Greybus System, which is inserted into exactly
      one slot in an Endoskeleton.  Modules exchange information one another
      and with the SVC via *unipro* Messages in accordance with the Greybus
      Specification.

MDK
    * Module Developers' Kit.

Module Developers' Kit
    * Project Ara Module Developer's Kit. This comprises various documents
      which collectively define the Ara platform.

Operation
    * An abstraction defined as part of a Protocol. An Operation comprises an
      Operation Type, an Operation Request (or simply "Request"), and an
      Operation Response (or simply "Response").

    * Requests and Responses are *unipro* Messages; the *unipro* L4 payload and
      semantics of each Request and Response are defined by the Greybus
      Specification.

Operation Type
    * Each Protocol defines a set of Operation Types. Each Operation Type has a
      name, a Request Value, and a Response Value.

    * An Operation Type has a name, along with a one-byte nonzero value, from
      which the Operation Type's Request Value and Response Value are derived.

    * Each Operation Type has an associated unsigned value, which lies in the
      range 1 to 127 (the value 0 is invalid). Each Operation Type has a
      Request Value, which equals the Operation Type's value, and a Response
      Value, which equals the Operation Type's value logically ORed with 0x80.

    * For example, an Operation Type with value 0x03 has Request Value 0x03,
      and Response Value 0x83.

Protocol
    * A Greybus Protocol defines the layout and semantics of the Operations
      which may be exchanged on a Connection.

    * Protocols are grouped according to their function:

        - Special Protocols
        - Device Class Protocols
        - Bridged PHY Protocols

Request
    * A *unipro* Message sent by a Module which initiates an Operation.

    * The *unipro* L4 payload and semantics of each Request are specified by
      the Protocol definition of the Request's associated Operation.

Requestor
    * Within the context of an Operation, the Module which sends or sent the
      Operation's Request.

Response
    * A *unipro* Message which is  an Operation.

    * The *unipro* L4 payload and semantics of each Response are specified by
      the Protocol definition of the Response's associated Operation.

Respondent
    * Within the context of an Operation, the Module which sends or sent the
      Operation's Response.

Special Protocol
    * A Greybus Protocol which permits discovery and enumeration of Modules by
      the SVC, and for other special-purpose tasks, such as network and power
      bus management.

Supervisory Controller (SVC)
    * An entity within the Endoskeleton that configures and controls each
      *unipro* Switch, as well as its power bus and wake/detect signals.
