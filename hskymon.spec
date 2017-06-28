Summary: Sky-Monitor for Subaru Telescope, NAOJ
Name: hskymon
Version: 2.9.7
Release: 1%{?_dist_release}
License: GPL3
Group: Applications/Engineering
Packager: Akito Tajitsu <tajitsu@naoj.org>
URL: https://www.naoj.org/Observing/Insruments/HDS/hskymon-e.html
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Vendor: Subaru Telescope, National Astoronomical Observatory of Japan
Requires:	gtk2 
Requires:	cairo
Requires:       libxml2
Requires:       openssl
BuildRequires:	gtk2-devel
BuildRequires:	cairo-devel
BuildRequires:  libxml2-devel
BuildRequires:  openssl-devel

%description
Sky-Monitor for Subaru Telescope, 
National Astronomical Observatory of Japan

%prep
%setup -q -n hskymon-%{version}

%build
./configure

make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf %{buildroot}

%files
/usr/local/bin/hskymon
/usr/local/share/man/man1/hskymon.1

%changelog
* Mon Jun 26 2017 Akito Tajitsu <tajitsu@naoj.org>
- first release for version 2.9.7
