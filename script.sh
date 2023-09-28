#!/bin/bash

# Update package lists
sudo apt-get update -y

# Upgrade installed packages
sudo DEBIAN_FRONTEND=noninteractive apt-get upgrade -y

# Add NGINX repository
echo | sudo add-apt-repository ppa:nginx/stable
sudo apt-get update

# Install NGINX
sudo DEBIAN_FRONTEND=noninteractive apt-get install nginx -y

# Display NGINX version
sudo nginx -v

# Install software-properties-common
sudo apt-get install software-properties-common

# Add Ondrej PHP repository
sudo echo | add-apt-repository ppa:ondrej/php
sudo apt-get update

# Install PHP 7.2 and necessary extensions
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y php7.2 php7.2-mysql php7.2-fpm php7.2-xml php7.2-gd php7.2-opcache php7.2-mbstring

# Install additional PHP packages
sudo DEBIAN_FRONTEND=noninteractive apt-get -y install zip unzip php7.2-zip

# Display PHP version
sudo php -v

# Install Composer
curl -sS https://getcomposer.org/installer | sudo php -- --install-dir=/usr/local/bin --filename=composer

# Create Laravel project
cd /var/www
composer create-project laravel/laravel test --prefer-dist --no-interaction
chown -R www-data:www-data test/
chmod -R 775 test/

# Configure NGINX for Laravel
cd /etc/nginx/sites-available
rm default
git clone https://github.com/Yokeshwer/php.git
cd php/
cp default /etc/nginx/sites-available/
cd ..
rm -r php/

# Restart NGINX
systemctl restart nginx
