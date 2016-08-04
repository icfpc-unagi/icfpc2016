SERVICE := ninemaster

start: mount check
	if ! docker top $(SERVICE) >/dev/null 2>/dev/null; then \
		set -e -x; \
		mkdir -p /alloc /mirror /ninemaster; \
		docker build --tag=local/$(SERVICE) .; \
		docker rm --force $(SERVICE) 2>/dev/null || true; \
		docker run \
		    --privileged \
		    --rm \
		    --name=$(SERVICE) \
		    --hostname=$(SERVICE) \
		    --volume=/alloc:/alloc \
		    --volume=/mirror:/mirror \
		    --volume=/ninemaster:/ninemaster \
		    --publish=80:80 \
		    --publish=2200:22 \
		    --publish=2201-2219:2201-2219 \
		    --publish=22000-22127:22000-22127 \
		    local/$(SERVICE); \
	fi
.PHONY: start

stop: check
	-docker kill $(SERVICE) 2>/dev/null
	-docker rm --force $(SERVICE) 2>/dev/null
.PHONY: stop

mount: check
	if ! mountpoint /alloc; then \
		mkdir -p /alloc; \
		sshfs -o allow_other,uid=65534,gid=65534 \
		    ninefs@ninefs.sx9.jp:/alloc /alloc; \
	fi
.PHONY: mount

unmount: check
	-/bin/fusermount -uz /alloc
	-umount -f /alloc
.PHONY: 

restart: check
	make stop
	make unmount
	cd /github && git pull
	make mount
	make start
.PHONY: restart

backup: check
	xz --stdout --compress /ninecluster/ninemaster.dmg > /ninecluster/ninemaster.dmg.xz
.PHONY: backup

install:
	test "$$(whoami)" = 'root'
	apt-get update -qq
	apt-get install -qqy \
	    apt-transport-https ca-certificates curl lxc iptables sshfs make
	curl -sSL https://get.docker.com/ | sh
	sed -i 's/^GRUB_CMDLINE_LINUX=".*"/GRUB_CMDLINE_LINUX="cgroup_enable=memory swapaccount=1"/' /etc/default/grub
	update-grub
	echo 'Host *' > ~/.ssh/config
	echo '  StrictHostKeyChecking no' >> ~/.ssh/config
	echo '  UserKnownHostsFile /dev/null' >> ~/.ssh/config
.PHONY: install

check:
	test "$$(whoami)" = 'root'
.PHONY: check
