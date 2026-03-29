{
  description = "Wolfpack Empire - a multi-player, client/server Internet based war game";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "empserver";
          version = "4.4.1-git";

          src = ./.;

          nativeBuildInputs = [
            pkgs.autoconf
            pkgs.automake
            pkgs.perl
            pkgs.groff
          ];

          buildInputs = [
            pkgs.readline
            pkgs.ncurses
          ];

          postPatch = ''
            patchShebangs bootstrap build-aux scripts src/scripts tests

            # Nix strips .git, so the build falls into the "tarball" code path
            # which requires .tarball-version and sources.mk to be present.
            echo "4.4.1" > .tarball-version

            # Generate sources.mk listing all source files (normally done by
            # `make` from `git ls-files` when building from a git checkout).
            echo "src := $(find . -type f \! -path './.git/*' | sed 's|^\./||' | sort | tr '\n' ' ')" > sources.mk
          '';

          preConfigure = ''
            ./bootstrap
          '';

          configureFlags = [ "--with-pthread" ];

          # GNU make is the default builder in nixpkgs stdenv
          enableParallelBuilding = true;

          # Server-based tests (info, smoke, etc.) start emp_server, which
          # requires both localhost networking and a proper init that reaps
          # zombie processes.  Neither is available in the Nix build sandbox.
          # Non-server tests (files, fairland) pass.
          doCheck = false;

          meta = with pkgs.lib; {
            description = "Wolfpack Empire server - a multi-player, client/server Internet based war game";
            homepage = "http://www.wolfpackempire.com/";
            license = licenses.gpl3Plus;
            platforms = platforms.unix;
          };
        };

        # Server tests need localhost networking and a proper init process.
        # Run with: nix build .#empserver-tests --option sandbox false
        packages.empserver-tests = self.packages.${system}.default.overrideAttrs (old: {
          name = "empserver-tests";
          doCheck = true;
          checkTarget = "check";
          __noChroot = true;
        });

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];
          packages = [
            pkgs.gdb
            pkgs.git
          ];
        };
      }
    );
}
