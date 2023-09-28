
resource "aws_vpc" "week9test" {
  cidr_block = "10.0.0.0/16"

  tags = {
    Name = "WeeklyTest"
  }
}

resource "aws_security_group" "web" {
  name        = "web-php-sg"
  description = "for inbound rules"
  vpc_id = aws_vpc.week9test.id

  ingress {
    from_port = 80
    to_port   = 80
    protocol  = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }
  ingress {
    from_port = 22
    to_port   = 22
    protocol  = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }
  egress {
    from_port = 0
    to_port   = 0
    protocol  = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

}

  resource "aws_subnet" "public" {
  vpc_id                 = aws_vpc.week9test.id
  cidr_block             = "10.0.1.0/24"
  availability_zone      = "ap-south-1a"
  map_public_ip_on_launch = true

  tags = {
    Name = "public-subnet"
  }
}
resource "aws_subnet" "private" {
  vpc_id                 = aws_vpc.week9test.id
  cidr_block             = "10.0.2.0/24"
  availability_zone      = "ap-south-1b"
  map_public_ip_on_launch = false
  tags = {
    Name = "private-subnet"
  }
}
resource "aws_internet_gateway" "weeklytest" {
  vpc_id = aws_vpc.week9test.id
  tags = {
    Name = "test-igw"
  }
}
resource "aws_route_table" "public" {
  vpc_id = aws_vpc.week9test.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.weeklytest.id
  }

  tags = {
    Name = "public-route-table"
  }
}

resource "aws_route_table_association" "public_rta" {
  subnet_id      = aws_subnet.public.id
  route_table_id = aws_route_table.public.id
}

resource "aws_launch_configuration" "weeklytest" {
  name_prefix          = "test-"
  image_id             = "ami-0f5ee92e2d63afc18"
  instance_type        = "t2.micro"
  security_groups      = [aws_security_group.web.id]
  associate_public_ip_address = true
}
resource "aws_autoscaling_group" "ags-weeklytest" {
  name                 = "test-asg"
  launch_configuration = aws_launch_configuration.weeklytest.name
  min_size             = 2
  max_size             = 5
  desired_capacity     = 2
  vpc_zone_identifier  = [aws_subnet.public.id]
}

resource "aws_lb" "phpweb" {
  name          = "web-lb"
  internal      = false
  load_balancer_type = "application"
  security_groups  = [aws_security_group.web.id]
  subnets          = [aws_subnet.public.id,aws_subnet.private.id]

  enable_http2     = true
}
  
resource "aws_lb_target_group" "webphp_tg" {
  name = "webphp-targetgroup"
  port = 80
  protocol = "HTTP"
  target_type = "instance"
  vpc_id  = aws_vpc.week9test.id

  health_check {
    enabled = true
    interval = 30
    protocol = "HTTP"
    path = "/"
    timeout = 5
    healthy_threshold = 2
    unhealthy_threshold = 2
  }
}
resource "aws_lb_listener" "webphp_list" {
  load_balancer_arn = aws_lb.phpweb.arn
  port  = 80
  protocol  = "HTTP"
  default_action {
    type  = "fixed-response"
    fixed_response {
        content_type = "text/plain"
        status_code = "200"
        }
    }
}

resource "aws_lb_listener_rule" "php_alb" {
 listener_arn = aws_lb_listener.webphp_list.arn
 action {
        type = "forward"
        target_group_arn = aws_lb_target_group.webphp_tg.arn
        }

 condition {
 path_pattern {
        values = ["/"]
  }
 }
}

resource "aws_autoscaling_attachment" "php_attachment" {
 lb_target_group_arn = aws_lb_target_group.webphp_tg.arn
 autoscaling_group_name = aws_autoscaling_group.ags-weeklytest.name
}
