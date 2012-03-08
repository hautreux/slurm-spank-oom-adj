Summary: Slurm spank plugin for OOM adjust 
Name: slurm-spank-oom-adj
Version: 0.1.0
Release: 1.cea
License: GPL
Group: System Environment/Base
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: slurm-devel
Requires: slurm

%description
oom-adj SLURM spank plugin enables to automatically adjust the OOM killer score
of applications launched using SLURM. A common usage scenario is when one wants
to ensure that SLURM application are the first candidate preventing system tasks
to be killed by unexpectedly growing user applications.

%prep
%setup -q

%build
%{__cc} -g -shared -fPIC -o oom-adj.so slurm-spank-oom-adj.c

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_libdir}
mkdir -p $RPM_BUILD_ROOT%{_libdir}/slurm
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/slurm
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/slurm/plugstack.conf.d
install -m 755 oom-adj.so $RPM_BUILD_ROOT%{_libdir}/slurm
install -m 644 plugstack.conf $RPM_BUILD_ROOT%{_sysconfdir}/slurm/plugstack.conf.d/oom-adj.conf.example

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/slurm/oom-adj.so
%config(noreplace) %{_sysconfdir}/slurm/plugstack.conf.d/oom-adj.conf.example

%changelog
* Mon May 09 2011 HAUTREUX Matthieu <matthieu.hautreux@cea.fr> -  0.1.0-1
- Initial release
