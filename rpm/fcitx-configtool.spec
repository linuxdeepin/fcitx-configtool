%define specrelease 1%{?dist}
%if 0%{?openeuler}
%define specrelease 1
%endif

Name:		fcitx-configtool
Version:	0.4.10.19
Release:    %{specrelease}
Summary:	Gtk+-based configuring tools for Fcitx
License:	GPLv2+
URL:		https://fcitx-im.org/wiki/Fcitx
Source0:	%{name}-%{version}.tar.xz

BuildRequires:	gcc
BuildRequires:	cmake, fcitx-devel, gettext, intltool, libxml2-devel
BuildRequires:	gtk2-devel, iso-codes-devel, libtool
BuildRequires:	gtk3-devel
Requires:	fcitx


%description
fcitx-config-gtk and fcitx-config-gtk3 are Gtk based configuring tools for
Fcitx.

%global debug_package %{nil}

%prep
%setup -q
patch -p1 < rpm/fix-windows-cannot-close-bug.patch

%build
mkdir -pv build
pushd build
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_INSTALL_PREFIX=/usr
make %{?_smp_mflags} VERBOSE=1

%install
pushd build
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="install -p"
popd

%find_lang %{name}

%files -f %{name}.lang
%doc README
%license COPYING
%{_bindir}/*

%changelog
* Tue Apr 6 2021 uoser <uoser@uniontech.com> - 0.4.10.19-7
- Initial Package