# NOte: org.kde.Platform//5.9 does not contain any QtSql drivers
# suport for flatpack is postponed

flatpak --user remote-add kde --from https://distribute.kde.org/kderuntime.flatpakrepo --if-not-exists
flatpak --user install kde org.kde.Platform//5.9
flatpak --user install kde org.kde.Sdk//5.9

chmod +x boost-configure
flatpak-builder --force-clean --repo=repo --subject="Build of TOra `date`" app tora.json

flatpak --user remote-add toraapp repo --no-gpg-verify
flatpak --user install toraapp org.tora.tora
flatpak run org.tora.tora
