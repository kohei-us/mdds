Name:           mdds
Version:        0.3.1
Release:        1
Url:            http://code.google.com/p/multidimalgorithm/
License:        MIT/X11
Source:         mdds_0.3.1.tar.bz2
Group:          Development/Libraries/C and C++
Summary:        A collection of multi-dimensional data structure and indexing algorithm
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       boost-devel >= 1.39

%description
This library provides a collection of multi-dimensional data structure and indexing
algorithm.  All data structures are available as C++ templates, hence this is a 
header-only library, with no shared library to link against.

Authors:
--------
    Kohei Yoshida <kyoshida@novell.com>

%package        devel
Url:            http://code.google.com/p/multidimalgorithm/
License:        MIT/X11
Group:          Development/Libraries/C and C++
Summary:        A collection of multi-dimensional data structure and indexing algorithm
%description    devel
This library provides a collection of multi-dimensional data structure and indexing
algorithm.  All data structures are available as C++ templates, hence this is a 
header-only library, with no shared library to link against.

Authors:
--------
    Kohei Yoshida <kyoshida@novell.com>

%prep
%setup -q -n %{name}_%{version}

%build
./configure --prefix=%buildroot/usr
%check

%install
make install

%clean
rm -rf %buildroot

%files devel
%defattr(-,root,root)
%dir /usr/include/mdds
%dir /usr/include/mdds/hash_container
%dir /usr/share/mdds-devel
%dir /usr/share/mdds-devel/src
%dir /usr/share/mdds-devel/example
%dir /usr/share/doc/packages/mdds-devel
%doc AUTHORS NEWS README
/usr/include/mdds/*.hpp
/usr/share/mdds-devel/Makefile
/usr/share/mdds-devel/src/*.cpp
/usr/share/mdds-devel/example/*

%changelog
* Thu Oct 14 2010 kyoshida@novell.com
- Updated to version 0.3.1.
* Fri May 5 2010 kyoshida@novell.com
- Updated to version 0.3.0.
* Fri Apr 9 2010 kyoshida@novell.com
- Updated to version 0.2.1.
* Fri Apr 9 2010 kyoshida@novell.com
- Initial package version 0.2.0.

  
