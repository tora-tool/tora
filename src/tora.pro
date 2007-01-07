TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

FORMS	= toaboutui.ui \
	tobrowserconstraintui.ui \
	tobrowserfilterui.ui \
	tobrowserindexui.ui \
	tobrowsertableui.ui \
	tochartalarmui.ui \
	tochartsetupui.ui \
	todatabasesettingui.ui \
	todebugchangeui.ui \
	todebugwatch.ui \
	todroptablespaceui.ui \
	toeditextensiongotoui.ui \
	toeditextensionsetupui.ui \
	toglobalsettingui.ui \
	tohelpaddfileui.ui \
	tohelpsetupui.ui \
	tolinechartsetupui.ui \
	tomessageui.ui \
	tonewconnectionui.ui \
	tooraclesettingui.ui \
	topreferencesui.ui \
	toresultcontentfilterui.ui \
	toresultlistformatui.ui \
	torollbackdialogui.ui \
	toscriptui.ui \
	tosearchreplaceui.ui \
	tosecurityquotaui.ui \
	tosecurityroleui.ui \
	tosecurityuserui.ui \
	tostoragedatafileui.ui \
	tostoragedefinitionui.ui \
	tostoragedialogui.ui \
	tostorageprefsui.ui \
	tostoragetablespaceui.ui \
	tosyntaxsetupui.ui \
	totemplateaddfileui.ui \
	totemplateeditui.ui \
	totemplatesetupui.ui \
	totoolsettingui.ui \
	totuningoverviewui.ui \
	totuningsettingui.ui \
	toworksheetsetupui.ui

