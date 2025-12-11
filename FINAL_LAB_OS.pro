TEMPLATE = subdirs

SUBDIRS += \
    Common \
    StudentApp \
    SupervisorApp
    SupervisorApp.depends = Common
    StudentApp.depends = Common
