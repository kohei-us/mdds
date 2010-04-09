Name:           mdds
Version:        0.2.0
Release:        1
Url:            http://code.google.com/p/multidimalgorithm/
License:        MIT/X11
Packager:       Kohei Yoshida  <kyoshida@novell.com>
Source:         mdds_0.2.0.tar.bz2
Group:          Development/Libraries/C++
Summary:        A collection of multi-dimensional data structure and indexing algorithm. 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
A collection of multi-dimensional data structure and indexing algorithm. 

Authors:
--------
    Kohei Yoshida <kyoshida@novell.com>

%package        devel
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
mkdir -p %buildroot/usr/share/mdds-%version/src
mkdir -p %buildroot/usr/share/doc/packages/mdds-%version
cp inc/*.hpp %buildroot/usr/include/mdds/
cp src/*.cpp %buildroot/usr/share/mdds-%version/src/
cp AUTHORS NEWS README %buildroot/usr/share/doc/packages/mdds-%version/

%install
install -d %buildroot/usr/include/mdds
install -d %buildroot/usr/share/mdds-%version

%clean
rm -rf %buildroot

%files devel
%doc AUTHORS NEWS README
%defattr(-,root,root)
%dir /usr/include/mdds
%dir /usr/share/mdds-%version
%dir /usr/share/mdds-%version/src
%dir /usr/share/doc/packages/mdds-%version
/usr/include/mdds/*.hpp
/usr/share/mdds-%version/src/*.cpp
/usr/share/doc/packages/mdds-%version/*

%changelog
* Fri Apr 9 2010 kyoshida@novell.com
- Initial package version 0.2.0.

  
