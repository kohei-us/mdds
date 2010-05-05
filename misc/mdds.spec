Name:           mdds
Version:        0.3.0
Release:        1
Url:            http://code.google.com/p/multidimalgorithm/
License:        MIT/X11
Packager:       Kohei Yoshida  <kyoshida@novell.com>
Source:         mdds_0.3.0.tar.bz2
Group:          Development/Libraries/C++
Summary:        A collection of multi-dimensional data structure and indexing algorithm. 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       boost-devel >= 1.39

%description
A collection of multi-dimensional data structure and indexing algorithm. 

Authors:
--------
    Kohei Yoshida <kyoshida@novell.com>

%package        devel
Url:            http://code.google.com/p/multidimalgorithm/
License:        MIT/X11
Packager:       Kohei Yoshida  <kyoshida@novell.com>
Group:          Development/Libraries/C++
Summary:        A collection of multi-dimensional data structure and indexing algorithm. 
%description    devel
A collection of multi-dimensional data structure and indexing algorithm. 

Authors:
--------
    Kohei Yoshida <kyoshida@novell.com>

%prep
%setup -q -n %{name}_%{version}

%build
mkdir -p %buildroot/usr/include/mdds
mkdir -p %buildroot/usr/share/mdds-devel/src
mkdir -p %buildroot/usr/share/mdds-devel/example
mkdir -p %buildroot/usr/share/doc/packages/mdds-devel
cp inc/mdds/*.hpp %buildroot/usr/include/mdds/
cp Makefile %buildroot/usr/share/mdds-devel/
cp src/*.cpp %buildroot/usr/share/mdds-devel/src/
cp example/* %buildroot/usr/share/mdds-devel/example/
cp AUTHORS NEWS README %buildroot/usr/share/doc/packages/mdds-devel/

%install
install -d %buildroot/usr/include/mdds
install -d %buildroot/usr/share/mdds-devel
install -d %buildroot/usr/share/doc/packages/mdds-devel

%clean
rm -rf %buildroot

%files devel
%defattr(-,root,root)
%dir /usr/include/mdds
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
* Fri May 5 2010 kyoshida@novell.com
- Updated to version 0.3.0.
* Fri Apr 9 2010 kyoshida@novell.com
- Updated to version 0.2.1.
* Fri Apr 9 2010 kyoshida@novell.com
- Initial package version 0.2.0.

  
