Summary: Sky-Monitor for Subaru Telescope, NAOJ
Name: hskymon
Version: 3.3.4
Release: 1%{?_dist_release}
License: GPL3
Group: Applications/Engineering
Packager: Akito Tajitsu <tajitsu@naoj.org>
URL: http://www.naoj.org/Observing/tools/hskymon
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Vendor: Subaru Telescope, National Astoronomical Observatory of Japan
Requires:	gtk2 
Requires:	cairo
Requires:       libxml2
Requires:       openssl
Requires:       json-c
BuildRequires:	gtk2-devel
BuildRequires:	cairo-devel
BuildRequires:  libxml2-devel
BuildRequires:  openssl-devel
BuildRequires:  json-c-devel

%description
hskymon is a GTK+2 based cross-platform application to monitor 
celestial objects, sky condition, and telescope in Subaru Telescope 
Gen2 system. It can be used as a visibility checker, 
a finding charts creator, and a guiding stars finder 
for your obs preparation in your environment 
as well as a monitoring software in Subaru/Gen2 system.
Basically, hskymon is optimized for Subaru Telescope. However,
it can be adopted to other alt-az mount telescopes, 
changing positional parameters.

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
