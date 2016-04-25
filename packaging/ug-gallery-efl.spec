
Name:       org.tizen.ug-gallery-efl
Summary:    Gallery UX
Version:    1.3.21
Release:    1
Group:      Applications/Multimedia Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  cmake
BuildRequires:  gettext-tools
BuildRequires:  edje-tools
BuildRequires:  prelink
BuildRequires:  libicu-devel
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(libtzplatform-config)

%description
Description: gallery UG

%define PREFIX    	 %{TZ_SYS_RO_APP}/%{name}
%define MANIFESTDIR      %{TZ_SYS_RO_PACKAGES}
%define ICONDIR          %{TZ_SYS_RO_ICONS}/default/small

%define RESDIR           %{PREFIX}/res
%define EDJDIR           %{RESDIR}/edje
%define IMGDIR           %{EDJDIR}/images
%define BINDIR           %{PREFIX}/bin
%define LIBDIR           %{PREFIX}/lib
%define LOCALEDIR        %{RESDIR}/locale
%define IMGDIRRES	 %{RESDIR}/res

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . \
    -DPREFIX=%{PREFIX}   \
    -DPKGDIR=%{name}     \
    -DIMGDIR=%{IMGDIR}   \
    -DEDJDIR=%{EDJDIR}   \
    -DPKGNAME=%{name}    \
    -DBINDIR=%{BINDIR}   \
    -DMANIFESTDIR=%{MANIFESTDIR}   \
    -DEDJIMGDIR=%{EDJIMGDIR}   \
    -DLIBDIR=%{LIBDIR}   \
    -DICONDIR=%{ICONDIR}   \
    -DLOCALEDIR=%{LOCALEDIR} \
    -DRESDIR=%{RESDIR}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{LIBDIR}

%post
GOPTION="-g 6514"

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%dir
%{LIBDIR}
%{BINDIR}/*
%{MANIFESTDIR}/*.xml
%{ICONDIR}/*
%{RESDIR}/*
%{ICONDIR}/*
#%{IMGDIR}/*
#%{LOCALEDIR}/*
