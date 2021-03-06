Summary: Plotting widgets for Gtkmm3
Name: gtkmm-plplot
Version: 2.5
Release:	1%{?dist}
License: LGPL
Group: System Environment/Libraries
Source: gtkmm-plplot-%{version}.tar.gz
URL: https://tschoonj.github.io/gtkmm-plplot
Requires: gtkmm30 >= 3.2.0
Requires: plplot-libs >= 5.10.0
BuildRequires:   gtkmm30-devel >= 3.2.0
BuildRequires:   plplot-devel >= 5.10.0
BuildRequires:   doxygen graphviz
BuildRequires:   gtkmm30-doc >= 3.2.0 cairomm-doc pangomm-doc atkmm-doc 
BuildRequires:   boost-devel
%if 0%{?rhel} != 7
BuildRequires:   meson
%endif

%description
Gtkmm3 widgets for scientific plotting, implemented as a wrapper around PLplot. Includes support for two- and three-dimensional plots, polar and contour plots. 

%package devel
Summary: Plotting widgets for Gtkmm3: development package
Group: Development/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: gtkmm30-devel
Requires: plplot-devel
Requires: pkgconfig 
Requires: boost-devel

%description devel
Gtkmm3 widgets for scientific plotting, implemented as a wrapper around PLplot. Includes support for two- and three-dimensional plots, polar and contour plots. 

This package contains the necessary headers and libraries to start developing with Gtkmm-PLplot.

%package doc
Summary: Plotting widgets for Gtkmm3: documentation
Group: Development/Libraries
Requires: %{name}%{?_isa}-devel = %{version}-%{release}
Requires: gtkmm30-doc cairomm-doc pangomm-doc atkmm-doc

%description doc
Gtkmm3 widgets for scientific plotting, implemented as a wrapper around PLplot. Includes support for two- and three-dimensional plots, polar and contour plots. 

This package contains the Gtkmm-PLplot documentation in html format.

%prep
%if 0%{?rhel} != 7
%setup -q -n %{name}-%{version}
%else
%setup -q
%endif

%build
%if 0%{?rhel} != 7
%meson
%meson_build
%else
%configure --enable-documentation
#necessary to fix rpath issues during rpmbuild
sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
make
%endif

%install
%if 0%{?rhel} != 7
%meson_install
%else
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
libtool --finish $RPM_BUILD_ROOT%{_libdir}
find $RPM_BUILD_ROOT -name '*.la' | xargs rm -f
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%if %{defined ldconfig_scriptlets}
%ldconfig_scriptlets
%else
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%endif

%files
%defattr(-, root, root)
%{_libdir}/libgtkmm-plplot*.so.*

%files devel
%defattr(-, root, root)
%{_libdir}/*so
%{_libdir}/pkgconfig/*
%{_libdir}/gtkmm-plplot*/*
%{_includedir}/*

%files doc
%defattr(-,root,root,-)
%{_datadir}/doc/gtkmm-plplot/*


%changelog
* Tue Apr 14 2020 Tom Schoonjans <Tom.Schoonjans@me.com>
- Add support for building with meson
* Fri Dec 13 2019 Tom Schoonjans <Tom.Schoonjans@me.com>
- Remove static libraries
* Wed Aug 10 2016 Tom Schoonjans <Tom.Schoonjans@me.com>
- Initial spec file

